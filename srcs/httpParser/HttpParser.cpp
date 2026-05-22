#include "HttpParser.hpp"
#include "HttpError.hpp"
#include "HttpStates.hpp"
#include "HttpToken.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "RequestLine.hpp"
#include "Server.hpp"
#include "StrView.hpp"
#include "Token.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <utility>

using std::make_pair;
using std::string;

const char *g_methods[] = {"DEFAULT", "GET", "POST", "DELETE"};

const char *const HttpParser::bodyLabels[STATE_SIZE]
	= {"REQUEST_LINE",		  "HEADERS",	   "VALIDATE",	   "BODY",
	   "SET_CHUNK_SIZE",	  "SET_BODY_SIZE", "CHUNKED_BODY", "NO_BODY",
	   "MAKE_ERROR_RESPONSE", "RETURN"};

// Public constructors and destructors
HttpParser::HttpParser(const Server &server, int fd) :
	_server(server),
	_fd(fd),
	_request(new Request(fd)),
	_requestLineParser(_token, _expect, _request->_requestLine, _state),
	_charRead(0),
	_headerLen(0),
	_state(REQUEST_LINE),
	_nextBodySection(0),
	_needsMoreInput(false),
	_toGetChunk(false),
	_token(_request->_buff.getBuffRef()),
	_expect(_token),
	_status(200) {}

HttpParser::~HttpParser() { delete _request; }

// Public Methods
void HttpParser::validateRequestLine() {
	const Location &location = _server.findLocation(_request->getPath());

	if (location.usingDefaultMethods())
		LOG(Logger::LOG, "Using default Methods");
	else if (!location.isAllowedMethod(_request->getMethod()))
		throw HttpError::HTTP_METHOD_NOT_ALLOWED;
	LOG_LABELED(Logger::LOG, "Allowed ", g_methods[_request->getMethod()]);

	struct stat file_status;
	if (OK
		!= stat(_request->_requestLine.getPath().getStr().c_str(),
				&file_status))
		throw HttpError::HTTP_NOT_FOUND;

	// bool isDir = S_ISDIR(file_status.st_mode);
	//
	// int permissionsToCheck = 0;
	// switch (_requestLine->getMethod()) {
	// case Location::GET:
	// 	permissionsToCheck = isDir ? (R_OK | X_OK) : R_OK;
	// 	break;
	// case Location::POST:
	// 	permissionsToCheck = isDir ? (W_OK | X_OK) : W_OK;
	// 	break;
	// case Location::DELETE:
	// 	// target = getParentDir(path);
	// 	permissionsToCheck = W_OK | X_OK;
	// }
}

void HttpParser::setError(const uint errorCode, const char *detailMsg) {
	_status = errorCode;
	_state = MAKE_ERROR_RESPONSE;
	LOGNUM_LABELED(Logger::WARNING, detailMsg,
				   " | Registered html error code: ", errorCode);
}

void HttpParser::validateKey(StrView Key) {
	switch (*Key.getStart()) {
	case ('H'):
		if (Key.compare("Host") && _request->getHeaderValue(Key))
			return setError(400, "Request has more than one host!");
		return;
	}
}

void HttpParser::parseHeaders() {
	while (1) {
		_token.loadNextOfType(Token::WORD, "Http header key or eof");
		StrView key = _token.getStrV();

		if (*key.getEnd() != ':')
			throw _token.parsingErr(":");
		key.trimEnd(1);

		_token.loadNextStr("http header Value");
		StrView value = _token.getStrV();

		_request->_headers.insert(make_pair(key, value));
		if (HttpToken::DOUBLE == _token.handleNewline())
			return;
	}
}

void HttpParser::validateRequest() {
	if (!_request->getHeaderValue("Host"))
		return setError(400, "Request has no Host!");
};

void HttpParser::setBodySize() {
	const StrView *bodyType = _request->getHeaderValue("Transfer-Encoding");
	if (bodyType && bodyType->compare("chunked")) {
		_state = SET_CHUNK_SIZE;
		return;
	}

	bodyType = _request->getHeaderValue("Content-Length");
	if (bodyType) {
		string size = bodyType->getStr();
		_nextBodySection = atoll(size.c_str());
		_state = BODY;
		return;
	}

	_state = RETURN;
}

void HttpParser::getChunk() {
	if (_token.getSizeLeft() < _nextBodySection) {
		_nextBodySection -= _token.getSizeLeft();
		_chunks.push_back(_token.getRemaining());
		_needsMoreInput = true;
		return;
	}

	_token.loadNextChunk(_nextBodySection);
	_nextBodySection = 0;
	_chunks.push_back(_token.getStrV());

	switch (_token.loadHttpNewLine()) {
	case (Token::ENDOFILE):
		_state = RETURN;
		return;
	case (Token::OTHER):
		_needsMoreInput = true;
	case (Token::NEWLINE):
		_state = SET_BODY_SIZE;
	}
}

Request *HttpParser::recvAndParse() {
	_needsMoreInput = false;

	if (!_request->_buff.recvAppend(_fd))
		return NULL;

	while (!_needsMoreInput) {
		switch (_state) {
		case (REQUEST_LINE):
			_requestLineParser.parse();
			LOG_OBJ("Parsed request line", _request->_requestLine);
			continue;
		case (HEADERS):
			parseHeaders();
			continue;
		case (VALIDATE):
			validateRequest();
		case (SET_BODY_SIZE):
			setBodySize();
			LOGNUM_LABELED(Logger::LOG, "Body mode set to ", bodyLabels[_state],
						   _state);
			continue;
		case (BODY):
			if (_token.getSizeLeft() < _nextBodySection)
				_needsMoreInput = true;
			else {
				_request->_body = _token.getRemaining();
				_state = RETURN;
			}
			continue;
		case (SET_CHUNK_SIZE):
			_needsMoreInput = _token.loadNextHex(&_nextBodySection);
			continue;
		case (CHUNKED_BODY):
			getChunk();
			continue;
		case (MAKE_ERROR_RESPONSE):
			return NULL;
		case (RETURN):
			Request *ret = _request;
			_request = new Request(_fd);
			return ret;
		}
	}
	return NULL;
}
