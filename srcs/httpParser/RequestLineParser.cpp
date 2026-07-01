#include "RequestLineParser.hpp"
#include "Expect.hpp"
#include "HttpError.hpp"
#include "HttpStates.hpp"
#include "HttpToken.hpp"
#include "Logger.hpp"
#include "RequestLine.hpp"
#include "RequestPathConsolidator.hpp"
#include <stdexcept>

using std::runtime_error;

// constructors and destructors
RequestLineParser::RequestLineParser(HttpToken &token, Expect &expect,
									 RequestLine &requestline,
									 uchar &mainState) :
	_token(token),
	_expect(expect),
	_requestLine(requestline),
	_state(METHOD),
	_mainState(mainState) {}

RequestLineParser::~RequestLineParser() {}

void RequestLineParser::parse() {
	try {
		switch (_state) {
		case METHOD:
			_token.loadNextOfType(Token::WORD, "Http Method");
			_requestLine._method = _expect.method();
			if (DEFAULT == _requestLine._method)
				throw HttpError(HttpStatus::BAD_REQUEST);
			_state = PATH;

		case PATH: // fallthrough
			_expect.path(&_rawPath);
			_requestLine._path = RequestPathConsolidator::consolidate(_rawPath);
			_state = HTTP;

		case HTTP: // fallthrough
			_token.loadNext();
			if (!_token.compare("HTTP/1.1")) {
				if (_token.compare("HTTP/1.0"))
					_requestLine._http1_1 = false;
				else
					throw HttpError(HttpStatus::VERSION_NOT_SUPPORTED);
			}

		case NEWLINE:
			switch (_token.handleNewline()) {
			case HttpToken::SINGLE:
				_mainState = VALIDATE;
				return;
			case HttpToken::DOUBLE:
				_mainState = RETURN;
			}
		}
	} catch (const runtime_error &err) {
		throw runtime_error(TRACED(err.what()));
	} catch (const HttpError &err) {
		LOG_LABELED(Logger::WARNING, "HttpError: ", err.what());
		throw;
	}
}
