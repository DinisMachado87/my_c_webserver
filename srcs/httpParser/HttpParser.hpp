#pragma once

#include "BufferManager.hpp"
#include "Expect.hpp"
#include "HttpHeadersParser.hpp"
#include "HttpStates.hpp"
#include "HttpToken.hpp"
#include "IOBuffer.hpp"
#include "Request.hpp"
#include "RequestLineParser.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "StrView.hpp"
#include <linux/stat.h>
#include <sys/types.h>
#include <vector>

#define BUFFER_SIZE 1024
#define NEEDS_MORE_INPUT true

/* Incremental HTTP/1.x request parser. Driven by recvAndParse() —
 * each call reads from the socket, feeds bytes to the tokenizer,
 * and advances the state machine as far as input allows.
 * Returns a completed Request* or NULL if more data is needed. */
class HttpParser
{
public:
	// Constructors and destructors
	HttpParser(const Server &server, int fd, BufferManager &bufferManager);
	~HttpParser();

	// Only public access.
	// Returns completed Request on success, NULL if more input needed.
	// Throws HttpError on protocol violations, ClientClosed on EOF.
	Request *recvAndParse();

private:
	const Server &_server;

	int _fd;
	BufferManager &_bufferManager;

	Request *_request;
	Response *_response;
	const Location *_location; // resolved during VALIDATE

	IOBuffer *_activeBuffer;
	HttpToken _token;
	Expect _expect;

	/* Sub-parsers — each owns its own resume state */
	RequestLineParser _requestLineParser;
	HttpHeadersParser _headersParser;

	ssize_t _charRead;
	ssize_t _headerLen;
	uchar _mainState; // current state (see HttpStates.hpp)
	uchar _subState;
	size_t _nextBodySection;

	std::vector<StrView> _chunks;
	bool _toGetChunk;

	uint _status;

	static const char *const bodyLabels[STATE_SIZE]; // for debug logging

	/* Internal steps */
	void createGetResponse();
	uchar handleNewline(uint singleNextState, uint doubleNextState);
	void validate();
	void setError(const uint errorCode, const char *detailMsg);
	void validateRequest();
	void validateKey(StrView Key);
	void validateHeader();
	void getChunk();
	void setBodySize();
	void receive(int fd);
	void parseHeaders();
	void parseRequestLine();

	/* Explicit disables */
	HttpParser();
	HttpParser &operator=(const HttpParser &other);
	HttpParser(const HttpParser &other);

	friend class HttpParserTest;
};
