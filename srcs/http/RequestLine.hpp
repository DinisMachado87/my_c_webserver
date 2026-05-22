#pragma once

#include "RequestPath.hpp"
class RequestLine {
private:
	RequestPath _path;
	uchar _method;
	bool _http1_1;

protected:
	friend class RequestLineParser;
	friend class Request;

	// Explicit Disables
	RequestLine();

public:
	// Constructors and destructors
	RequestLine(RequestPath path, uchar method, bool http1_1);
	~RequestLine();

	// Operators overload

	// Getters and setters

	// Methods
	uchar getMethod() const;
	const StrView &getPathQuery() const;
	const StrView &getPathFragment() const;
	const StrView &getPath() const;
	const char *getMethodStr() const;
	void print(std::ostream &stream) const;
};

std::ostream &operator<<(std::ostream &os, const RequestLine &req);
