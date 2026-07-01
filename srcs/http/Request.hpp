#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "IOBuffer.hpp"
#include "Location.hpp"
#include "RequestLine.hpp"
#include "RequestPath.hpp"
#include "StrView.hpp"
#include "StrViewMap.hpp"
#include "webServ.hpp"
#include <sys/types.h>

/* Parsed HTTP request. Owns the IOBuffer that backs all StrViews
 * (request line, headers, body). Built incrementally by HttpParser
 * as data arrives. */
class Request
{
private:
	/* explicit disables */
	Request(const Request &other);

protected:
	IOBuffer _buff;

	RequestLine _requestLine;
	StrViewMap _headers;
	StrView _body;

	Location *_location; // resolved after VALIDATE state
	std::string _absolutePath;

	// Friend only private constructor
	Request(int fd, BufferManager &bufferManager);
	friend class HttpParser;
	friend class Expect;

public:
	~Request();

	// Operators overload
	Request &operator=(const Request &other);

	void validateRequest();
	const char *getMethodStr() const;
	void print(std::ostream &stream) const;

	const StrView *getHeaderValue(const char *charKey) const; // NULL if missing
	const StrView *getHeaderValue(StrView &key) const;

	RequestPath &getPath();
	uchar getMethod() const;
	const StrView &getBody() const;
};

std::ostream &operator<<(std::ostream &os, const Request &req);

#endif
