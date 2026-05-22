#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "RequestBuffer.hpp"
#include "RequestLine.hpp"
#include "StrView.hpp"
#include "StrViewMap.hpp"
#include "webServ.hpp"
#include <string>
#include <sys/types.h>

class Request {
private:
	// explicit disables
	Request(const Request &other);

protected:
	RequestBuffer _buff;

	RequestLine _requestLine;
	StrViewMap _headers;
	StrView _body;

	friend class HttpParser;
	friend class Expect;
	// private constructor
	Request(int fd);

public:
	// Constructors and destructors
	~Request();

	// Operators overload
	Request &operator=(const Request &other);

	// Methods
	const char *getMethodStr() const;
	void print(std::ostream &stream) const;

	// Getters and setters
	const StrView *getHeaderValue(const char *charKey) const;
	const StrView *getHeaderValue(StrView &key) const;
	const StrView &getPath() const;
	uchar getMethod() const;
	const StrView &getBody() const;
};

std::ostream &operator<<(std::ostream &os, const Request &req);

#endif
