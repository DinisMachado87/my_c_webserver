#include "Request.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <cstring>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

using std::ostream;
using std::string;
using std::stringstream;

// Public constructors and destructors
Request::Request(int fd) :
	_buff(fd) {}

Request::~Request() {}

// Public Methods

uchar Request::getMethod() const { return _requestLine.getMethod(); };
const StrView &Request::getPath() const { return _requestLine.getPath(); };

const StrView *Request::getHeaderValue(const char *key) const {
	StrView strv = key;
	return _headers.find(strv);
};
const StrView *Request::getHeaderValue(StrView &key) const {
	return _headers.find(key);
};

void Request::print(std::ostream &stream) const {
	stream << _requestLine;
	stream << _headers;
}

ostream &operator<<(ostream &os, const Request &req) {
	req.print(os);
	return os;
}

const StrView &Request::getBody() const { return _body; }
