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
	RequestLineParser(HttpToken &token, Expect &expect,
					  RequestLine &requestline, uchar &mainState);
	~RequestLineParser();

	// Advances as far as input allows. Throws HttpError on protocol violations.
	void parse();

private:
	friend class HttpParser;
	/* Explicit disables*/
	RequestLineParser();
	RequestLineParser(const RequestLineParser &other);

protected:
	HttpToken &_token;
	Expect &_expect;
	RequestLine &_requestLine;

	StrView _rawPath;

	uchar _state;
	uchar &_mainState;
	enum e_state { METHOD, PATH, HTTP, NEWLINE };
};
