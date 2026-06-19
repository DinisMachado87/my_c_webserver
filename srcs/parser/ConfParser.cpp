#include "ConfParser.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "Overrides.hpp"
#include "Server.hpp"
#include "StrView.hpp"
#include "Token.hpp"
#include "webServ.hpp"
#include <arpa/inet.h>
#include <cctype>
#include <climits>
#include <cstddef>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

using std::map;
using std::ostringstream;
using std::pair;
using std::runtime_error;
using std::string;
using std::vector;
typedef pair<map<uint, StrView>::iterator, bool> errorVecPair;

// Public constructors and destructors
ConfParser::ConfParser(string &configStr, vector<Server *> &servers,
					   vector<StrView> &defaultsVecBuff) :
	_servers(servers),
	_newServer(new Server()),
	_newLocation(_newServer->_strvVecBuf),
	_defaultsVecBuff(defaultsVecBuff),
	_vecCursor(0),
	_token(Token::configDelimiters(), configStr.c_str(), configStr.size()),
	_expect(_token) {};

ConfParser::~ConfParser() {}

/* CONFIG STRUCTURE
 *
 * server {						<- nextserver()
 *		listen address;			<- parseServerLine()->parseListen()
 * 		serverKey value;		<- parseServerLine()
 *		overridesKey value;		<- parseServerLine()->parseOverrides()
 * 		...
 * 		location path {				<- parseLocation()
 *			locationKey value;		<- parseLocationLine()
 *			overridesKey value;		<- parseLocationLine()->parseOverrides()
 * 			...
 * 		}
 * 		serverKey value;		<- parseServerLine()
 * 		...
 * 		location path {...}			<- parseLocation()
 * 		...
 * }
 * */

// Main controlflow
void ConfParser::createServers() {
	while (Token::WORD == _token.loadNext() && _token.compare("server"))
		nextServer();
	if (_token._type != Token::ENDOFILE)
		throw parsingErr("\"server\"");
	LOG(Logger::LOG, "Done Parsing");
}

void ConfParser::nextServer() {
	_token.loadNextOfType(Token::OPENBLOCK, "{");
	while (1) {
		switch (_token.loadNext()) {
		case Token::WORD:
			if (_token.compare("location"))
				parseLocation();
			else
				parseServerLine();
			continue;
		case Token::CLOSEBLOCK:
			consolidateAndStoreNewServer();
			_newServer = new Server();
			return;
		default:
			throw parsingErr("Unexpected token");
		}
	}
}

// Parse Structures
void ConfParser::parseServerLine() {
	if (_token.compare("listen"))
		parseListen();
	else if (!parseOverrides(_newServer->_defaults._overrides))
		throw parsingErr("Unknown directive");
}

void ConfParser::parseLocation() {
	_expect.path(&_newLocation._path);
	_token.loadNextOfType(Token::OPENBLOCK, "{");
	while (1) {
		_token.loadNext();
		switch (_token.getType()) {
		case Token::WORD:
			parseLocationline();
			continue;
		case Token::CLOSEBLOCK:
			_newServer->_locations.push_back(_newLocation);
			_newLocation = Location(_newServer->_strvVecBuf);
			return;
		case Token::ENDOFILE:
			throw parsingErr("}");
		}
	}
}

void ConfParser::parseLocationline() {
	Location &loc = _newLocation;

	if (_token.compare("allowed_methods")) {
		parseMethod(loc._overrides);
		return;
	} else if (_token.compare("return")) {
		loc._returnCode = _expect.nextInteger();
		_expect.path(&loc._returnPath);
	} else if (_token.compare("rewrite")) {
		_expect.path(&loc._rewrite_old);
		_expect.path(&loc._rewrite_new);
	} else if (_token.compare("upload_enable"))
		loc._uploadEnable = _expect.onOff();
	else if (_token.compare("upload_path"))
		_expect.path(&loc._uploadPath);
	else if (_token.compare("cgi_extension")) {
		loc._cgiExtensions
			= _expect.wordVec(_newServer->_strvVecBuf, _vecCursor);
		return;
	} else if (_token.compare("cgi_path")) {
		loc._cgiPath = _expect.wordVec(_newServer->_strvVecBuf, _vecCursor);
		return;
	} else if (parseOverrides(loc._overrides))
		return;
	else
		throw parsingErr("Unknown directive");

	_token.loadNextOfType(Token::SEMICOLON, "';'");
}

bool ConfParser::parseOverrides(Overrides &ov) {
	const bool isOverridesParam = true;

	if (_token.compare("root")) {
		_expect.path(&ov._root);
		ov._set |= 1 << F_ROOT;
	} else if (_token.compare("autoindexing")) {
		ov._autoindex = _expect.onOff();
		ov._set |= 1 << F_AUTOINDEX;
	} else if (_token.compare("index")) {
		ov._index = _expect.wordVec(_newServer->_strvVecBuf, _vecCursor);
		ov._set |= 1 << F_INDEX;
		return true;
	} else if (_token.compare("client_max_body_size")) {
		ov._clientMaxBody = _expect.size();
		ov._set |= 1 << F_CLIENT_BODY;
	} else if (_token.compare("error_page")) {
		_expect.errorPage(ov._error);
		ov._set |= 1 << F_ERROR;
	} else if (_token.compare("allowed_methods")) {
		parseMethod(ov);
		return isOverridesParam;
	} else
		return !isOverridesParam;

	_token.loadNextOfType(Token::SEMICOLON, "';'");
	return isOverridesParam;
}

// Parse elements
void ConfParser::parseMethod(Overrides &ov) {
	uchar method = DEFAULT;
	while (1) {
		_token.loadNext();
		switch (_token.getType()) {
		case Token::WORD:
			method = _expect.method();
			if (!method)
				throw parsingErr("Unknown method");
			ov._allowedMethods |= (1 << method);
			break;
		case Token::SEMICOLON:
			if (ov._allowedMethods != DEFAULT) {
				ov._set |= F_METHODS;
				return;
			}
		default: // fallthrough
			throw parsingErr("Method definition");
		}
	}
}

void ConfParser::parseListen() {
	_token.loadNextOfType(Token::WORD, "listen address");

	Listen listen;
	string portStr = _token.getString();
	string ipStr = "*";

	// in case ip:port extracts ip
	size_t colonPos = portStr.find(':');
	if (colonPos != string::npos) {
		ipStr = portStr.substr(0, colonPos);
		portStr = portStr.substr(colonPos + 1);
	}

	listen._host = _expect.ip(ipStr);
	listen._port = _expect.port(portStr);

	_token.loadNextOfType(Token::SEMICOLON, "';'");
	_newServer->_listen.push_back(listen);
}

// Consolidate
static inline void consolidateStrv(StrView &strv, char *&dest) {
	if (strv.size() == 0)
		return;
	strv.consolidate(dest);
	dest += strv.size();
}

static inline void consolidateMap(map<uint, StrView> &errors, char *&dest) {
	map<uint, StrView>::iterator cur = errors.begin();
	map<uint, StrView>::iterator end = errors.end();
	for (; cur != end; ++cur)
		consolidateStrv(cur->second, dest);
}

void ConfParser::consolidatelocation(Location &loc, char *&dest) {
	// location StrViews
	consolidateStrv(loc._path, dest);
	consolidateStrv(loc._returnPath, dest);
	consolidateStrv(loc._rewrite_old, dest);
	consolidateStrv(loc._rewrite_new, dest);
	consolidateStrv(loc._uploadPath, dest);
	// _overrides StrViews
	consolidateStrv(loc._overrides._root, dest);
	consolidateMap(loc._overrides._error, dest);
}

void ConfParser::consolidateAndStoreNewServer() {
	// instantiate temporary program defaults
	uchar progDefSet = (1 << F_ROOT) | (1 << F_INDEX) | (1 << F_AUTOINDEX)
					   | (1 << F_CLIENT_BODY) | (1 << F_METHODS);
	Overrides programDefaults = Overrides(_defaultsVecBuff, progDefSet);
	// reserve and get new consolidation buffer pointer
	size_t size = _token.getStrBuffSize();
	_newServer->_strBuf.resize(size);
	char *dest = &_newServer->_strBuf[0];

	// consolidate vector buffer of all spans
	vector<StrView> &tokensVec = _newServer->_strvVecBuf;
	for (size_t i = 0; i < tokensVec.size(); i++)
		consolidateStrv(tokensVec[i], dest);

	// server inherit from program
	Overrides &serverOverrides = _newServer->_defaults._overrides;
	consolidatelocation(_newServer->_defaults, dest);
	serverOverrides.inheritUnsetParams(programDefaults);

	// locations inherit from server
	vector<Location> &locations = _newServer->_locations;
	for (size_t i = 0; i < locations.size(); i++) {
		Overrides &locOverrides = locations[i]._overrides;
		consolidatelocation(locations[i], dest);
		locOverrides.inheritUnsetParams(serverOverrides);

		if (locations[i]._cgiExtensions.len() != locations[i]._cgiPath.len())
			throw runtime_error("Error parsing location: diferent number "
								"of cgi extentions and paths");
	}

	if (_newServer->_defaults._overrides._root == "")
		throw runtime_error("Cannot init server. No root defined.");

	_servers.push_back(_newServer);
	_token._strVBuffSize = 0;
	_vecCursor = 0;
}

// Err Handeling
std::runtime_error ConfParser::parsingErr(const char *expected) const {
	ostringstream oss;
	oss << "Error Parsing config: "
		<< "Expected \"" << expected << "\" "
		<< "got \"" << _token.getString() << "\" ";
	return std::runtime_error(oss.str());
}
