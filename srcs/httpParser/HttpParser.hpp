#pragma once

#include "BufferManager.hpp"
#include "Expect.hpp"
#include "HttpHeadersParser.hpp"
#include "HttpStates.hpp"
#include "HttpToken.hpp"
#include "RequestLineParser.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include <linux/stat.h>
#include <sys/types.h>
#include <vector>

#define BUFFER_SIZE 1024
#define NEEDS_MORE_INPUT true

class HttpParser {
private:
	const Server &_server;

	int _fd;
	BufferManager &_bufferManager;
	Request *_request;
	Response *_response;

	HttpToken _token;
	Expect _expect;

	RequestLineParser _requestLineParser;
	HttpHeadersParser _headersParser;

	ssize_t _charRead;
	ssize_t _headerLen;
	uchar _state;
	uchar _subState;
	size_t _nextBodySection;

	std::vector<StrView> _chunks;
	bool _toGetChunk;

	uint _status;

	static const char *const bodyLabels[STATE_SIZE];

	// Methods
	uchar handleNewline(uint singleNextState, uint doubleNextState);
	void validateRequestLine();
	void setError(const uint errorCode, const char *detailMsg);
	void validateRequest();
	void validateKey(StrView Key);
	void validateHeader();
	void getChunk();
	void setBodySize();
	void receive(int fd);
	void parseHeaders();
	void parseRequestLine();

	// Static initializer for Token class
	static const uchar *delimiters();
	// Explicit disables
	HttpParser();
	HttpParser &operator=(const HttpParser &other);
	HttpParser(const HttpParser &other);

	friend class HttpParserTest;

public:
	// Constructors and destructors
	HttpParser(const Server &server, int fd, BufferManager &bufferManager);
	~HttpParser();
	// Methods
	Request *recvAndParse();
};
