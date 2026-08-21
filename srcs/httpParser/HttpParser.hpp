#pragma once

#include "BufferManager.hpp"
#include "Expect.hpp"
#include "HttpHeadersParser.hpp"
#include "HttpToken.hpp"
#include "Request.hpp"
#include "RequestLineParser.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include <sys/types.h>

class Response;

/* Incremental HTTP/1.x parser. Driven by recvAndParse() —
 * reads from socket, feeds bytes to tokenizer, advances the
 * state machine as far as input allows.
 * Returns a Response* when complete, NULL if more data needed.
 * State machine: REQUEST_LINE → HEADERS → ROUTE.
 * Body reading is not the parser's concern — it moves to
 * Response::readBody(), driven by epoll after route(). */
class HttpParser
{
private:
	/* state */
	const Server &_server;
	BufferManager &_bufferManager;
	int _clientFd;
	uchar _state;

	/* parse in place data structure */
	Request *_request;

	/* Tokeninzer */
	HttpToken _token;
	Expect _expect;

	/* Sub-parsers — each owns its own resume state */
	RequestLineParser _requestLineParser;
	HttpHeadersParser _headersParser;

	/* Explicit disables */
	HttpParser();
	HttpParser &operator=(const HttpParser &other);
	HttpParser(const HttpParser &other);

	void reset();

	friend class HttpParserTest;

public:
	HttpParser(const Server &server, int fd, BufferManager &bufferManager);
	~HttpParser();

	// Returns Response* on complete request, NULL if more input needed.
	// Throws ClientClosed on EOF.
	Response *recvAndParse();
};
