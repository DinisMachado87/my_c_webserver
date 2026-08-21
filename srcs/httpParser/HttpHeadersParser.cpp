#include "HttpHeadersParser.hpp"
#include "HttpToken.hpp"
#include "webServ.hpp"
#include <utility>

using std::make_pair;

HttpHeadersParser::HttpHeadersParser(StrViewMap &headers, HttpToken &token,
									 Expect &expect) :
	_token(token),
	_expect(expect),
	_state(KEY),
	_headers(headers)
{
}

HttpHeadersParser::~HttpHeadersParser() {}

bool HttpHeadersParser::parseHeaders()
{
	while (true) {
		switch (_state) {
		case KEY:
			// if new line end of headers
			if (_token.isNewLine())
				return _token.consumeNewLine(); // DONE
			if (_token.needsMoreInput())
				return ONGOING;

			if (!_token.loadNextWord())
				return ONGOING;
			_key = _token.getStrV();
			if (*_key.end() != ':')
				throw _token.parsingErr(":");
			_key.removeSuffix(1);
			_state = VALUE;
			// fallthrough

		case VALUE:
			_token.loadNextStr("http header Value");
			if (_token.needsMoreInput())
				return ONGOING;
			_value = _token.getStrV();
			_headers.insert(make_pair(_key, _value));
			_state = NEWLINE;
			// fallthrough

		case NEWLINE:
			if (!_token.consumeNewLine())
				return ONGOING;
			_state = KEY;
			break;
		}
	}
}
