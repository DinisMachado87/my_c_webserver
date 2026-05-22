#ifndef HTTPERROR_HPP
#define HTTPERROR_HPP

#include <exception>
#include <sys/types.h>

class HttpError : public std::exception {
	uint _status;
	const char *_msg;

public:
	HttpError(uint status, const char *msg);
	HttpError(const HttpError &base, const char *detail);

	uint getStatus() const;
	const char *what() const throw();

	// 2xx
	static const HttpError HTTP_OK;
	static const HttpError HTTP_CREATED;
	static const HttpError HTTP_NO_CONTENT;
	// 3xx
	static const HttpError HTTP_MOVED_PERMANENTLY;
	static const HttpError HTTP_FOUND;
	// 4xx
	static const HttpError HTTP_BAD_REQUEST;
	static const HttpError HTTP_FORBIDDEN;
	static const HttpError HTTP_NOT_FOUND;
	static const HttpError HTTP_METHOD_NOT_ALLOWED;
	static const HttpError HTTP_REQUEST_TIMEOUT;
	static const HttpError HTTP_CONFLICT;
	static const HttpError HTTP_GONE;
	static const HttpError HTTP_LENGTH_REQUIRED;
	static const HttpError HTTP_CONTENT_TOO_LARGE;
	static const HttpError HTTP_URI_TOO_LONG;
	static const HttpError HTTP_UNSUPPORTED_MEDIA_TYPE;
	// 5xx
	static const HttpError HTTP_INTERNAL_SERVER_ERROR;
	static const HttpError HTTP_NOT_IMPLEMENTED;
	static const HttpError HTTP_BAD_GATEWAY;
	static const HttpError HTTP_SERVICE_UNAVAILABLE;
	static const HttpError HTTP_GATEWAY_TIMEOUT;
	static const HttpError HTTP_VERSION_NOT_SUPPORTED;
};

#endif
