#ifndef HTTPSTATUS_HPP
#define HTTPSTATUS_HPP

#include <ostream>
#include <webServ.hpp>

class HttpStatus
{
private:
	uint _code;
	const char *_msg;

public:
	// Constructors and destructors
	HttpStatus(uint code, const char *reason);
	HttpStatus(const HttpStatus &other);
	HttpStatus &operator=(const HttpStatus &other);

	// Methods
	std::ostream &print(std::ostream &stream) const;
	uint getCode() const;
	const char *getReason() const;

	// 2xx
	static const HttpStatus _OK;
	static const HttpStatus CREATED;
	static const HttpStatus NO_CONTENT;
	// 3xx
	static const HttpStatus MOVED_PERMANENTLY;
	static const HttpStatus FOUND;
	// 4xx
	static const HttpStatus BAD_REQUEST;
	static const HttpStatus FORBIDDEN;
	static const HttpStatus NOT_FOUND;
	static const HttpStatus METHOD_NOT_ALLOWED;
	static const HttpStatus REQUEST_TIMEOUT;
	static const HttpStatus CONFLICT;
	static const HttpStatus GONE;
	static const HttpStatus LENGTH_REQUIRED;
	static const HttpStatus CONTENT_TOO_LARGE;
	static const HttpStatus URI_TOO_LONG;
	static const HttpStatus UNSUPPORTED_MEDIA_TYPE;
	// 5xx
	static const HttpStatus INTERNAL_SERVER_ERROR;
	static const HttpStatus NOT_IMPLEMENTED;
	static const HttpStatus BAD_GATEWAY;
	static const HttpStatus SERVICE_UNAVAILABLE;
	static const HttpStatus GATEWAY_TIMEOUT;
	static const HttpStatus VERSION_NOT_SUPPORTED;
};

inline std::ostream &operator<<(std::ostream &stream, const HttpStatus &status)
{
	status.print(stream);
	return stream;
}

#endif
