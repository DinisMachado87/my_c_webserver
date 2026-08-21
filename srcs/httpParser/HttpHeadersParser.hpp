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
	HttpHeadersParser(StrViewMap &headers, HttpToken &token, Expect &expect);
	~HttpHeadersParser();

	bool parseHeaders();

private:
	/* Explicit disables*/
	HttpHeadersParser(const HttpHeadersParser &other);
	HttpHeadersParser();

protected:
	enum e_httpHeadersState { KEY, SEPARATOR, VALUE, NEWLINE };

	HttpToken &_token;
	Expect &_expect;

	uchar _state;

	StrViewMap _headers;
	StrView _key;
	StrView _value;
};

#endif
