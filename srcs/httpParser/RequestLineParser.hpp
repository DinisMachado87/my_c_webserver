#pragma once

#include "Expect.hpp"
#include "HttpToken.hpp"
#include "RequestLine.hpp"
#include "StrView.hpp"

class RequestLineParser {
private:
	friend class HttpParser;
	// Explicit disables
	RequestLineParser();
	RequestLineParser(const RequestLineParser &other);
	// RequestLineParser &operator=(const RequestLineParser &other);

protected:
	HttpToken &_token;
	Expect &_expect;
	RequestLine &_requestLine;

	StrView _rawPath;

	uchar _state;
	uchar &_mainState;
	enum e_state { METHOD, PATH, HTTP, NEWLINE };

public:
	// Constructors and destructors
	RequestLineParser(HttpToken &token, Expect &expect,
					  RequestLine &requestline, uchar &mainState);
	~RequestLineParser();
	// Methods
	void parse();
};
