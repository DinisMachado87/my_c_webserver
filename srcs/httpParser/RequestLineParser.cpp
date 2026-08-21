#include "RequestLineParser.hpp"
#include "Expect.hpp"
#include "HttpError.hpp"
#include "HttpToken.hpp"
#include "Logger.hpp"
#include "RequestLine.hpp"
#include "RequestPathConsolidator.hpp"
#include "Traced.hpp"
#include "webServ.hpp"
#include <stdexcept>

using std::runtime_error;

RequestLineParser::RequestLineParser(RequestLine &requestline, HttpToken &token,
									 Expect &expect) :
	_requestLine(requestline),
	_token(token),
	_expect(expect),
	_state(METHOD)
{
}

RequestLineParser::~RequestLineParser() {}

bool RequestLineParser::parse()
{
	try {
		switch (_state) {
		case METHOD:
			if (!_token.loadNextWord())
				return ONGOING;
			_requestLine._method = _expect.method();
			if (DEFAULT == _requestLine._method)
				throw HttpError(HttpStatus::BAD_REQUEST);
			_state = PATH;
			// fallthrough

		case PATH:
			_expect.path(&_rawPath);
			if (_token.needsMoreInput())
				return ONGOING;
			_requestLine._path = RequestPathConsolidator::consolidate(_rawPath);
			_state = HTTP;
			// fallthrough

		case HTTP:
			if (!_token.loadNextWord())
				return ONGOING;
			if (!_token.compare("HTTP/1.1")) {
				if (_token.compare("HTTP/1.0"))
					_requestLine._http1_1 = false;
				else
					throw HttpError(HttpStatus::VERSION_NOT_SUPPORTED);
			}
			_state = NEWLINE;
			// fallthrough

		case NEWLINE:
			return _token.consumeNewLine();
		}
	} catch (const runtime_error &err) {
		throw runtime_error(TRACED(err.what()));
	} catch (const HttpError &err) {
		LOG_LABELED(Logger::WARNING, "HttpError: ", err.what());
		throw;
	}
	return DONE;
}
