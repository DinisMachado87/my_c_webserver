#include "HttpError.hpp"

HttpError::HttpError(uint status, const char *msg) :
	_status(status),
	_msg(msg) {}

HttpError::HttpError(const HttpError &base, const char *detail) :
	_status(base._status),
	_msg(detail) {}

uint HttpError::getStatus() const { return _status; }
const char *HttpError::what() const throw() { return _msg; }

// 2xx
const HttpError HttpError::HTTP_OK(200, "OK");
const HttpError HttpError::HTTP_CREATED(201, "Created");
const HttpError HttpError::HTTP_NO_CONTENT(204, "No Content");
// 3xx
const HttpError HttpError::HTTP_MOVED_PERMANENTLY(301, "Moved Permanently");
const HttpError HttpError::HTTP_FOUND(302, "Found");
// 4xx
const HttpError HttpError::HTTP_BAD_REQUEST(400, "Bad Request");
const HttpError HttpError::HTTP_FORBIDDEN(403, "Forbidden");
const HttpError HttpError::HTTP_NOT_FOUND(404, "Not Found");
const HttpError HttpError::HTTP_METHOD_NOT_ALLOWED(405, "Method Not Allowed");
const HttpError HttpError::HTTP_REQUEST_TIMEOUT(408, "Request Timeout");
const HttpError HttpError::HTTP_CONFLICT(409, "Conflict");
const HttpError HttpError::HTTP_GONE(410, "Gone");
const HttpError HttpError::HTTP_LENGTH_REQUIRED(411, "Length Required");
const HttpError HttpError::HTTP_CONTENT_TOO_LARGE(413, "Content Too Large");
const HttpError HttpError::HTTP_URI_TOO_LONG(414, "URI Too Long");
const HttpError
	HttpError::HTTP_UNSUPPORTED_MEDIA_TYPE(415, "Unsupported Media Type");
// 5xx
const HttpError HttpError::HTTP_INTERNAL_SERVER_ERROR(500,
													  "Internal Server Error");
const HttpError HttpError::HTTP_NOT_IMPLEMENTED(501, "Not Implemented");
const HttpError HttpError::HTTP_BAD_GATEWAY(502, "Bad Gateway");
const HttpError HttpError::HTTP_SERVICE_UNAVAILABLE(503, "Service Unavailable");
const HttpError HttpError::HTTP_GATEWAY_TIMEOUT(504, "Gateway Timeout");
const HttpError
	HttpError::HTTP_VERSION_NOT_SUPPORTED(505, "HTTP Version Not Supported");
