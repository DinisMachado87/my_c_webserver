#pragma once

#include "HttpStatus.hpp"
#include <exception>
#include <ostream>

/* Throwable HTTP error — carries a status code and optional detail.
 * Caught by the parser to build error responses. */
class HttpError : public std::exception
{
	HttpStatus _status;
	const char *_detail;

public:
	HttpError(const HttpStatus &status);
	HttpError(const HttpStatus &status, const char *detail);
	HttpError(const HttpError &other);
	HttpError &operator=(const HttpError &other);

	const HttpStatus &getStatus() const;
	uint getCode() const;
	const char *what() const throw();
	const std::ostream &print(std::ostream &stream) const;
};

inline std::ostream &operator<<(std::ostream &stream, const HttpError &error)
{
	error.print(stream);
	return stream;
}
