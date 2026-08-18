#include "HttpStatus.hpp"

using std::ostream;

HttpStatus::HttpStatus(uint code, const char *reason) :
	_code(code),
	_msg(reason)
{
}

HttpStatus::HttpStatus(const HttpStatus &other) :
	_code(other._code),
	_msg(other._msg)
{
}

HttpStatus &HttpStatus::operator=(const HttpStatus &other)
{
	if (this != &other) {
		_code = other._code;
		_msg = other._msg;
	}
	return *this;
}

uint HttpStatus::getCode() const { return _code; }
const char *HttpStatus::getReason() const { return _msg; }
ostream &HttpStatus::print(ostream &stream) const
{
	stream << _code << " " << _msg;
	return stream;
}

// 2xx
const HttpStatus HttpStatus::_OK(200, "OK");
const HttpStatus HttpStatus::CREATED(201, "Created");
const HttpStatus HttpStatus::NO_CONTENT(204, "No Content");
// 3xx
const HttpStatus HttpStatus::MOVED_PERMANENTLY(301, "Moved Permanently");
const HttpStatus HttpStatus::FOUND(302, "Found");
// 4xx
const HttpStatus HttpStatus::BAD_REQUEST(400, "Bad Request");
const HttpStatus HttpStatus::FORBIDDEN(403, "Forbidden");
const HttpStatus HttpStatus::NOT_FOUND(404, "Not Found");
const HttpStatus HttpStatus::METHOD_NOT_ALLOWED(405, "Method Not Allowed");
const HttpStatus HttpStatus::REQUEST_TIMEOUT(408, "Request Timeout");
const HttpStatus HttpStatus::CONFLICT(409, "Conflict");
const HttpStatus HttpStatus::GONE(410, "Gone");
const HttpStatus HttpStatus::LENGTH_REQUIRED(411, "Length Required");
const HttpStatus HttpStatus::CONTENT_TOO_LARGE(413, "Content Too Large");
const HttpStatus HttpStatus::URI_TOO_LONG(414, "URI Too Long");
const HttpStatus HttpStatus::UNSUPPORTED_MEDIA_TYPE(415,
													"Unsupported Media Type");
// 5xx
const HttpStatus HttpStatus::INTERNAL_SERVER_ERROR(500,
												   "Internal Server Error");
const HttpStatus HttpStatus::NOT_IMPLEMENTED(501, "Not Implemented");
const HttpStatus HttpStatus::BAD_GATEWAY(502, "Bad Gateway");
const HttpStatus HttpStatus::SERVICE_UNAVAILABLE(503, "Service Unavailable");
const HttpStatus HttpStatus::GATEWAY_TIMEOUT(504, "Gateway Timeout");
const HttpStatus
	HttpStatus::VERSION_NOT_SUPPORTED(505, "HTTP Version Not Supported");
