#pragma once

#include "Expect.hpp"
#include "HttpToken.hpp"
#include "RequestLine.hpp"
#include "StrView.hpp"

/* Incrementally parses "METHOD /path HTTP/1.x\r\n".
 * Resumes from _state if the token stream runs dry mid-line. */
class RequestLineParser
{
public:
	RequestLineParser(RequestLine &requestline, HttpToken &token,
					  Expect &expect);
	~RequestLineParser();

	bool parse();

private:
	friend class HttpParser;
	/* Explicit disables*/
	RequestLineParser();
	RequestLineParser(const RequestLineParser &other);

protected:
	RequestLine &_requestLine;

	HttpToken &_token;
	Expect &_expect;

	StrView _rawPath;

	uchar _state;
	enum e_state { METHOD, PATH, HTTP, NEWLINE };
};
