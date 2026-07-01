#include "HttpParser.hpp"
#include "BufferManager.hpp"
#include "HttpError.hpp"
#include "HttpStates.hpp"
#include "HttpStatus.hpp"
#include "HttpToken.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "RedirectResponse.hpp"
#include "Request.hpp"
#include "RequestLine.hpp"
#include "RequestPath.hpp"
#include "Server.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>

using std::string;

const char *g_methods[] = {"DEFAULT", "GET", "POST", "DELETE"};

const char *const HttpParser::bodyLabels[STATE_SIZE]
	= {"REQUEST_LINE",		  "HEADERS",	   "VALIDATE",	   "BODY",
	   "SET_CHUNK_SIZE",	  "SET_BODY_SIZE", "CHUNKED_BODY", "NO_BODY",
	   "MAKE_ERROR_RESPONSE", "RETURN"};

// Public constructors and destructors
HttpParser::HttpParser(const Server &server, int fd,
					   BufferManager &bufferManager) :
	_server(server),
	_fd(fd),
	_bufferManager(bufferManager),
	_request(new Request(fd, bufferManager)),
	_location(NULL),
	_token(NULL),
	_expect(_token),
	_requestLineParser(_token, _expect, _request->_requestLine, _mainState),
	_headersParser(&_request->_headers, _mainState, _token, _expect),
	_charRead(0),
	_headerLen(0),
	_mainState(REQUEST_LINE),
	_nextBodySection(0),
	_toGetChunk(false),
	_status(200)
{
}

HttpParser::~HttpParser() { delete _request; }

// Main control flow
Request *HttpParser::recvAndParse()
{
	StrView read = _request->_buff.readIn();
	if (read.empty())
		return NULL;

	_token.loadParsingString(read);
	_token.resetNeedsMoreInputFlag();
	while (!_token.needsMoreInput()) {
		switch (_mainState) {
		case (REQUEST_LINE):
			_requestLineParser.parse();
			LOG_OBJ("Parsed request line:", _request->_requestLine);
			continue;
		case (HEADERS):
			_headersParser.parseHeaders(_mainState);
			LOG_OBJ("Parsed Headers:", _request->_headers);
			continue;
		case (VALIDATE):
			validate();
		case (SET_BODY_SIZE):
			setBodySize();
			LOGNUM_LABELED(Logger::LOG, "Body mode set to ",
						   bodyLabels[_mainState], _mainState);
			continue;
		case (BODY):
			_request->_body = _token.getRemaining();
			_mainState = RETURN; // to change to next state
			continue;
		case (SET_CHUNK_SIZE):
			// _needsMoreInput = _token.loadNextHex(&_nextBodySection);
			continue;
		case (CHUNKED_BODY):
			getChunk();
			continue;
		case (MAKE_ERROR_RESPONSE):
			return NULL;
		case (RETURN):
			Request *ret = _request;
			_request = new Request(_fd, _bufferManager);
			return ret;
		}
	}
	return NULL;
}

// Public Methods
void HttpParser::validate()
{
	if (!_request->getHeaderValue("Host"))
		return setError(400, "Request has no Host!");

	RequestPath &path = _request->getPath();
	_location = &_server.findLocation(path.path());

	if (!_location->isAllowedMethod(_request->getMethod()))
		throw HttpError(HttpStatus::METHOD_NOT_ALLOWED);
	LOG_LABELED(Logger::LOG, "Allowed ", g_methods[_request->getMethod()]);

	// validate Path
	_request->_absolutePath = _location->getRoot() + path.path();
	struct stat file_status;
	if (OK != stat(_request->_absolutePath.c_str(), &file_status)) {
		if ((errno == ENOENT) && (errno == ENOTDIR))
			throw HttpError(HttpStatus::NOT_FOUND);
		else if (errno == EACCES)
			throw HttpError(HttpStatus::FORBIDDEN);
		else
			throw HttpError(HttpStatus::INTERNAL_SERVER_ERROR);
	}
	bool isDirInSystem = S_ISDIR(file_status.st_mode);
	if (isDirInSystem && path.getType()) {
		_response = new RedirectResponse(_request);
		_mainState = BODY;
		return;
	} else if (!S_ISREG(file_status.st_mode))
		throw HttpError(HttpStatus::FORBIDDEN);

	if (path.getType() == RequestPath::EXECUTABLE) {
		const StrView &cgiExt = _location->findCgiPath(path.getCgiExtension());
		if (cgiExt.empty())
			path.setType(RequestPath::FILE);
	}

	int permissionsToCheck = 0;
	uchar method = _request->_requestLine.getMethod();
	switch (method) {
	case GET:
		permissionsToCheck = isDirInSystem ? (R_OK | X_OK) : R_OK;
		return createGetResponse();
	case POST:
		permissionsToCheck = isDirInSystem ? (W_OK | X_OK) : W_OK;
		break;
	case DELETE:
		// target = getParentDir(path);
		permissionsToCheck = W_OK | X_OK;
	}
	LOGNUM(Logger::LOG, "permissions to check: ", permissionsToCheck);

	_mainState++;
}

void HttpParser::createGetResponse()
{
	// const uchar type = _request->_requestLine.requestPath().getType();
	// switch (type) {
	// case RequestPath::DIR:
	// 	// get indexes
	// _request->_absolutePath +=
	//
	// 	case RequestPath::FILE:;
	// }
}

void HttpParser::setError(const uint errorCode, const char *detailMsg)
{
	_status = errorCode;
	_mainState = MAKE_ERROR_RESPONSE;
	LOGNUM_LABELED(Logger::WARNING, detailMsg,
				   " | Registered html error code: ", errorCode);
}

void HttpParser::setBodySize()
{
	const StrView *bodyType = _request->getHeaderValue("Transfer-Encoding");
	if (bodyType && bodyType->compare("chunked")) {
		_mainState = SET_CHUNK_SIZE;
		return;
	}

	bodyType = _request->getHeaderValue("Content-Length");
	if (bodyType) {
		string size = bodyType->getStr();
		_nextBodySection = atoll(size.c_str());
		_mainState = BODY;
		return;
	}

	_mainState = RETURN;
}

void HttpParser::getChunk()
{
	if (_token.getSizeLeft() < _nextBodySection) {
		_nextBodySection -= _token.getSizeLeft();
		_chunks.push_back(_token.getRemaining());
		return;
	}

	_token.loadNextChunk(_nextBodySection);
	_nextBodySection = 0;
	_chunks.push_back(_token.getStrV());

	// switch (_token.loadHttpNewLine()) {
	// case (Token::ENDOFILE):
	// 	_state = RETURN;
	// 	return;
	// case (Token::NEWLINE):
	// 	_state = SET_BODY_SIZE;
	// }
}
