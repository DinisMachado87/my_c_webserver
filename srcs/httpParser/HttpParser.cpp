#include "HttpParser.hpp"
#include "BufferManager.hpp"
#include "ClientClosed.hpp"
#include "ERRORResponse.hpp"
#include "HttpError.hpp"
#include "HttpStates.hpp"
#include "HttpToken.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "RequestLine.hpp"
#include "Response.hpp"
#include "Router.hpp"
#include "StrView.hpp"
#include "webServ.hpp"

const char *g_methods[] = {"DEFAULT", "GET", "POST", "DELETE"};

// Constructors
HttpParser::HttpParser(const Server &server, int fd,
					   BufferManager &bufferManager) :
	_server(server),
	_bufferManager(bufferManager),
	_clientFd(fd),
	_state(REQUEST_LINE),
	_request(new Request(fd, bufferManager)),
	_token(StrView()),
	_expect(_token),
	_requestLineParser(_request->_requestLine, _token, _expect),
	_headersParser(_request->_headers, _token, _expect)
{
}

HttpParser::~HttpParser() { delete _request; }

Response *HttpParser::recvAndParse()
{
	try {
		ssize_t n = _request->_buff.readIn();
		if (n == RequestBuffer::CARRY_OVERFLOW)
			throw HttpError(_state == REQUEST_LINE
								? HttpStatus::URI_TOO_LONG
								: HttpStatus::CONTENT_TOO_LARGE);
		if (n < 0)
			return NULL; // EAGAIN/error — wait next epoll

		StrView in = _request->_buff.unparsed();
		if (in.empty()) {
			if (_request->_buff.inClosed())
				throw ClientClosed();
			return NULL;
		}
		_token.loadParsingString(in);
		_token.resetNeedsMoreInputFlag();

		switch (_state) {
		case REQUEST_LINE:
			if (!_requestLineParser.parse())
				break;
			LOG_OBJ("Parsed request line:", _request->_requestLine);
			_state = HEADERS;
			// fallthrough

		case HEADERS:
			if (!_headersParser.parseHeaders())
				break;
			LOG_OBJ("Parsed Headers:", _request->_headers);
			_state = ROUTE;
			// fallthrough

		case ROUTE: {
			StrView leftover = _token.getRemaining();
			_request->_buff.consumed(in.size() - leftover.size());
			Response *resp = Router::route(_server, _clientFd, _bufferManager,
										   _request, leftover);
			reset(); // route succeeded, took ownership of _request
			return resp;
		}
		}
		_request->_buff.consumed(in.size() - _token.leftover().size());
		return NULL;

	} catch (const HttpError &e) {
		LOG_LABELED(Logger::WARNING, "HttpError: ", e.what());
		Request *request = _request;
		reset();
		return new ERRORResponse(e.getStatus(), request);
	}
}

void HttpParser::reset()
{
	_request = new Request(_clientFd, _bufferManager);

	_requestLineParser.~RequestLineParser(); // manual destructor call
	new (&_requestLineParser)				 // placement new - avoids copy
		RequestLineParser(_request->_requestLine, _token, _expect);

	_headersParser.~HttpHeadersParser();
	new (&_headersParser)
		HttpHeadersParser(_request->_headers, _token, _expect);

	_state = REQUEST_LINE;
}
