#ifndef HTTPHEADERSPARSER_HPP
#define HTTPHEADERSPARSER_HPP

#include "Expect.hpp"
#include "HttpToken.hpp"
#include "StrView.hpp"
#include "StrViewMap.hpp"
#include "webServ.hpp"

class HttpHeadersParser {
private:
	// Explicit disables
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

public:
	// Constructors and destructors
	HttpHeadersParser(StrViewMap *headers, uchar &mainState, HttpToken &token,
					  Expect &expect);
	~HttpHeadersParser();
	// Methods
	void parseHeaders(const uchar curState);
};

#endif
