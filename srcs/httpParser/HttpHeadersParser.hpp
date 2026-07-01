#ifndef HTTPHEADERSPARSER_HPP
#define HTTPHEADERSPARSER_HPP

#include "Expect.hpp"
#include "HttpToken.hpp"
#include "StrView.hpp"
#include "StrViewMap.hpp"
#include "webServ.hpp"

/* Incrementally parses "Key: Value\r\n" pairs until a double CRLF.
 * Advances _mainState when the header block ends. Resumes from
 * _state if input runs dry mid-header. */
class HttpHeadersParser
{
public:
	HttpHeadersParser(StrViewMap *headers, uchar &mainState, HttpToken &token,
					  Expect &expect);
	~HttpHeadersParser();

	void parseHeaders(const uchar curState);

private:
	/* Explicit disables*/
	HttpHeadersParser(const HttpHeadersParser &other);
	HttpHeadersParser &operator=(const HttpHeadersParser &other);
	HttpHeadersParser();

protected:
	enum e_httpHeadersState { KEY, SEPARATOR, VALUE, LINEEND };

	HttpToken &_token;
	Expect &_expect;

	uchar _state;
	uchar &_mainState;

	StrViewMap *_headers;
	StrView _key;
	StrView _value;
};

#endif
