#ifndef HTTPHEADERS_HPP
#define HTTPHEADERS_HPP

#include "StrViewMap.hpp"

class HttpHeaders {
private:
	// explicit disables
	HttpHeaders(const HttpHeaders &other);
	HttpHeaders &operator=(const HttpHeaders &other);

protected:
	StrViewMap _headers;

public:
	// Constructors and destructors
	HttpHeaders();
	~HttpHeaders();

	// Operators overload

	// Getters and setters

	// Methods
};

#endif
