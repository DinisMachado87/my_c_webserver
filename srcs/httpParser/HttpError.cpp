#include "HttpError.hpp"
#include <ostream>

using std::ostream;

HttpError::HttpError(const HttpStatus &status) :
	_status(status),
	_detail(status.getReason())
{
}

HttpError::HttpError(const HttpStatus &status, const char *detail) :
	_status(status),
	_detail(detail)
{
}

HttpError::HttpError(const HttpError &other) :
	std::exception(other),
	_status(other._status),
	_detail(other._detail)
{
}

HttpError &HttpError::operator=(const HttpError &other)
{
	if (this != &other) {
		_status = other._status;
		_detail = other._detail;
	}
	return *this;
}

const ostream &HttpError::print(ostream &stream) const
{
	stream << "HttpError: " << _detail << _status;
	return stream;
}

const HttpStatus &HttpError::getStatus() const { return _status; }
uint HttpError::getCode() const { return _status.getCode(); }
const char *HttpError::what() const throw() { return _detail; }
