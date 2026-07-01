#include "HttpHeadersParser.hpp"
#include "HttpToken.hpp"
#include "webServ.hpp"
#include <utility>

using std::make_pair;

// Public constructors and destructors
HttpHeadersParser::HttpHeadersParser(StrViewMap *headers, uchar &mainState,
									 HttpToken &token, Expect &expect) :
	_token(token),
	_expect(expect),
	_state(KEY),
	_mainState(mainState),
	_headers(headers) {}

HttpHeadersParser::~HttpHeadersParser() {}

// Public Methods

void HttpHeadersParser::parseHeaders(const uchar curState) {
	while (!_token.needsMoreInput() && _mainState == curState) {
		switch (_state) {
		case KEY:
			_token.loadNextOfType(Token::WORD, "Http header key or eof");
			if (_token.needsMoreInput())
				return;
			_state = VALUE;
			_key = _token.getStrV();

			if (*_key.end() != ':')
				throw _token.parsingErr(":");
			_key.removeSuffix(1);

		case VALUE:
			_token.loadNextStr("http header Value");
			if (_token.needsMoreInput())
				return;
			_state = LINEEND;
			_value = _token.getStrV();

			switch (_token.handleNewline()) {
			case HttpToken::ENDOFILE:
				return;
			case HttpToken::DOUBLE:
				_headers->insert(make_pair(_key, _value));
				_state = KEY;
				_mainState++;
				return;
			case HttpToken::SINGLE: // fallthrough
				_headers->insert(make_pair(_key, _value));
				_state = KEY;
			}
		}
	}
}
