#include "HttpStatus.hpp"

using std::ostream;

HttpStatus::HttpStatus(uint code, const char *codeMsgStr) :
	_code(code),
	_codeMsgStr(codeMsgStr)
{
}

HttpStatus::HttpStatus(const HttpStatus &other) :
	_code(other._code),
	_codeMsgStr(other._codeMsgStr)
{
}

HttpStatus &HttpStatus::operator=(const HttpStatus &other)
{
	if (this != &other) {
		_code = other._code;
		_codeMsgStr = other._codeMsgStr;
	}
	return *this;
}

uint HttpStatus::getCode() const { return _code; }
const StrView &HttpStatus::codeAndMsg() const { return _codeMsgStr; }
ostream &HttpStatus::print(ostream &stream) const
{
	stream << _codeMsgStr;
	return stream;
}

// 2xx
const HttpStatus HttpStatus::_OK(200, "200 OK");
const HttpStatus HttpStatus::CREATED(201, "201 Created");
const HttpStatus HttpStatus::NO_CONTENT(204, "204 No Content");
// 3xx
const HttpStatus HttpStatus::MOVED_PERMANENTLY(301, "301 Moved Permanently");
const HttpStatus HttpStatus::FOUND(302, "302 Found");
// 4xx
const HttpStatus HttpStatus::BAD_REQUEST(400, "400 Bad Request");
const HttpStatus HttpStatus::FORBIDDEN(403, "403 Forbidden");
const HttpStatus HttpStatus::NOT_FOUND(404, "404 Not Found");
const HttpStatus HttpStatus::METHOD_NOT_ALLOWED(405, "405 Method Not Allowed");
const HttpStatus HttpStatus::REQUEST_TIMEOUT(408, "408 Request Timeout");
const HttpStatus HttpStatus::CONFLICT(409, "409 Conflict");
const HttpStatus HttpStatus::GONE(410, "410 Gone");
const HttpStatus HttpStatus::LENGTH_REQUIRED(411, "411 Length Required");
const HttpStatus HttpStatus::CONTENT_TOO_LARGE(413, "413 Content Too Large");
const HttpStatus HttpStatus::URI_TOO_LONG(414, "414 URI Too Long");
const HttpStatus
	HttpStatus::UNSUPPORTED_MEDIA_TYPE(415, "415 Unsupported Media Type");
// 5xx
const HttpStatus HttpStatus::INTERNAL_SERVER_ERROR(500,
												   "500 Internal Server Error");
const HttpStatus HttpStatus::NOT_IMPLEMENTED(501, "501 Not Implemented");
const HttpStatus HttpStatus::BAD_GATEWAY(502, "502 Bad Gateway");
const HttpStatus HttpStatus::SERVICE_UNAVAILABLE(503,
												 "503 Service Unavailable");
const HttpStatus HttpStatus::GATEWAY_TIMEOUT(504, "504 Gateway Timeout");
const HttpStatus
	HttpStatus::VERSION_NOT_SUPPORTED(505, "505 HTTP Version Not Supported");
