#include "RequestPath.hpp"
#include "StrView.hpp"
#include <iostream>

using std::ostream;

RequestPath::RequestPath() :
	Path(),
	_isDir(false),
	_isCgi(false),
	_cgiExtension() {}

RequestPath::RequestPath(const StrView &pathStr) :
	Path(pathStr),
	_isDir(false),
	_isCgi(false),
	_cgiExtension() {}

RequestPath::RequestPath(const StrView &path, const StrView &query,
						 const StrView &fragment, const bool isDir,
						 const bool isCgi, const StrView &cgiExtension) :
	Path(path, query, fragment),
	_isDir(isDir),
	_isCgi(isCgi),
	_cgiExtension(cgiExtension) {}

bool RequestPath::isDir() const { return _isDir; }
bool RequestPath::isCgi() const { return _isCgi; }
const StrView &RequestPath::getCgiExtension() const { return _cgiExtension; }

void RequestPath::print(ostream &stream) const {
	Path::print(stream);
	stream << " isDir: " << _isDir << " isCgi: " << _isCgi
		   << " cgiExt: " << _cgiExtension;
}

ostream &operator<<(ostream &os, const RequestPath &path) {
	path.print(os);
	return os;
}
