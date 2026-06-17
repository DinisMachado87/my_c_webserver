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
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

using std::map;
using std::pair;
using std::runtime_error;
using std::string;
using std::vector;
typedef pair<map<uint, StrView>::iterator, bool> errorVecPair;

// Public constructors and destructors
ConfParser::ConfParser(string &configStr, vector<Server *> &servers,
					   const Location &programDefaults) :
	_programDefaults(programDefaults),
	_servers(servers),
	_newServer(new Server()),
	_newLocation(_newServer->_strvVecBuf),
	_vecCursor(0),
	_token(Token::configDelimiters(), configStr.c_str(), configStr.size()),
	_expect(_token) {
	_newServer->reserve(configStr.length() * 0.6, 10, 10);
};

ConfParser::~ConfParser() {}

// Err Handeling
std::runtime_error ConfParser::parsingErr(const char *expected) const {
	std::ostringstream oss;
	oss << "Error Parsing config: "
		<< "Expected \"" << expected << "\" "
		<< "got \"" << _token.getString() << "\" "
		<< "in line " << _token.getLineN() << "\"";

	return std::runtime_error(oss.str());
}

// Private Methods
void ConfParser::parseMethod(Overrides &ov) {
	uchar method = DEFAULT;
	while (1) {
		_token.loadNext();
		switch (_token.getType()) {
		case Token::WORD:
			method = _expect.method();
			if (!method) throw parsingErr("Unknown method");
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

bool ConfParser::parseOverrides(Overrides &ov) {
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
		return true;

	} else
		return false;

	_token.loadNextOfType(Token::SEMICOLON, "';'");
	return true;
}

void ConfParser::parseLocationParam() {
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

void ConfParser::parseLocation() {
	_expect.path(&_newLocation._path);
	_token.loadNextOfType(Token::OPENBLOCK, "{");

	while (1) {
		_token.loadNext();

		switch (_token.getType()) {

		case Token::CLOSEBLOCK:
			if (_newLocation._cgiExtensions.len()
				!= _newLocation._cgiPath.len())
				throw runtime_error("Error parsing location: diferent number "
									"of cgi extentions and paths");
			_newServer->_locations.push_back(_newLocation);
			_newLocation = Location(_newServer->_strvVecBuf);
			return;

		case Token::WORD:
			parseLocationParam();
			continue;

		case Token::ENDOFILE:
			throw parsingErr("}");
		}
	}
}

void ConfParser::parseServerLine() {
	LOG(Logger::LOG, "Parsing location");
	if (_token.compare("listen")) {
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
	} else if (!parseOverrides(_newServer->_serverDefaults._overrides))
		throw parsingErr("Unknown directive");
}

void ConfParser::inheritUnsetParameters() {
	// _serverDefaults inherit unset params from _programDefaults
	Overrides &serverDefaults = _newServer->_serverDefaults._overrides;
	serverDefaults.mergeFrom(_programDefaults._overrides);

	// Locations inherit unset params from _serverDefaults
	for (size_t i = 0; i < _newServer->_locations.size(); ++i) {
		Overrides &locParams = _newServer->_locations[i]._overrides;
		locParams.mergeFrom(serverDefaults);
	}
}

void ConfParser::nextServer() {
	LOG_TITLE("Parsing new server");
	while (1) {
		switch (_token.loadNext()) {

		case Token::WORD:
			if (_token.compare("location"))
				parseLocation();
			else
				parseServerLine();
			continue;

		case Token::CLOSEBLOCK:
			consolidateBuffers();
			inheritUnsetParameters();
			if (_newServer->_serverDefaults._overrides._root == "")
				throw runtime_error("Cannot init server. No root defined.");

			_servers.push_back(_newServer);
			_newServer = new Server();
			_vecCursor = 0;
			return;

		default:
			throw parsingErr("Unexpected token");
		}
	}
}

void ConfParser::createServers() {
	while (1) {
		switch (_token.loadNext()) {

		case Token::WORD:
			if (_token.compare("server")) {
				_token.loadNextOfType(Token::OPENBLOCK, "{");
				nextServer();
				break;
			} else
				throw parsingErr("\"server\"");

		case Token::ENDOFILE:
			LOG(Logger::LOG, "Done Parsing");
			return;

		default:
			throw parsingErr("{");
		}
	}
}

// Consolidate
static inline void consolidateStrv(StrView &sv, char *&dest) {
	sv.consolidate(dest);
	dest += sv.size();
}

static inline void consolidateMap(map<uint, StrView> &errors, char *&dest) {
	map<uint, StrView>::iterator cur = errors.begin();
	map<uint, StrView>::iterator end = errors.end();
	for (; cur != end; ++cur)
		consolidateStrv(cur->second, dest);
}

void ConfParser::consolidateBuffer() {
	size_t size = _token.getStrBuffSize();
	string &newBuff = _newServer->_strBuf;
	newBuff.resize(size);
	char *dest = &newBuff[0];

	vector<StrView *> &tokens = _token._tokensInUse;
	for (size_t i = 0; i < tokens.size(); i++)
		consolidateStrv(*tokens[i], dest);

	vector<StrView> &tokensVec = _newServer->_strvVecBuf;
	for (size_t i = 0; i < tokensVec.size(); i++)
		consolidateStrv(tokensVec[i], dest);

	consolidateMap(_newServer->_serverDefaults._overrides._error, dest);

	vector<Location> &locations = _newServer->_locations;
	for (size_t i = 0; i < locations.size(); i++)
		consolidateMap(locations[i]._overrides._error, dest);

	_token.resetConsolidationCounters();
}
