#pragma once

#include "RequestPath.hpp"

class RequestLine
{
private:
	RequestPath _path;
	uchar _method;
	bool _http1_1;

protected:
	friend class RequestLineParser;
	friend class Request;

	/* Explicit Disables */
	RequestLine();

public:
	RequestLine(RequestPath path, uchar method, bool http1_1);
	~RequestLine();

	uchar getMethod() const;
	const StrView &getPathQuery() const;
	const StrView &getPathFragment() const;
	RequestPath &requestPath();
	const char *getMethodStr() const;
	void print(std::ostream &stream) const;
};

std::ostream &operator<<(std::ostream &os, const RequestLine &req);
