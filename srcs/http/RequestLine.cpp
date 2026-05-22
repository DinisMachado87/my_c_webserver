#include "RequestLine.hpp"
#include "StrView.hpp"
#include "webServ.hpp"

// Public constructors and destructors
RequestLine::RequestLine() {}

RequestLine::RequestLine(RequestPath path, uchar method, bool http1_1) :
	_path(path),
	_method(method),
	_http1_1(http1_1) {}

RequestLine::~RequestLine() {}

// Public Methods

uchar RequestLine::getMethod() const { return _method; };
const StrView &RequestLine::getPath() const { return _path.getPath(); }
const StrView &RequestLine::getPathFragment() const {
	return _path.getFragment();
}
const StrView &RequestLine::getPathQuery() const { return _path.getQuery(); }

// Print Methods
const char *RequestLine::getMethodStr() const { return g_methods[_method]; };

void RequestLine::print(std::ostream &stream) const {
	stream << "Method: " << getMethodStr();
	stream << " | " << _path << "\n";
}

std::ostream &operator<<(std::ostream &os, const RequestLine &reqLine) {
	reqLine.print(os);
	return os;
}
