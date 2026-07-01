#include "RequestPath.hpp"
#include "StrView.hpp"
#include <iostream>

using std::ostream;

RequestPath::RequestPath() :
	Path(),
	_type(NONE) {}

RequestPath::RequestPath(const StrView &pathStr) :
	Path(pathStr),
	_type(NONE) {}

RequestPath::RequestPath(const StrView &path, const StrView &query,
						 const StrView &fragment, const uchar type,
						 const StrView &dirPath, const StrView &sufix) :
	Path(path, query, fragment),
	_type(type),
	_dirPath(dirPath),
	_sufix(sufix) {}

const char *RequestPath::typeLabels[SIZE] = {
#define type(valueStr) #valueStr,
	TYPE_LIST
#undef type
};

void RequestPath::setType(uchar type) { _type = type; }
uchar RequestPath::getType() const { return _type; }

const StrView &RequestPath::getCgiExtension() const { return _sufix; }

void RequestPath::print(ostream &stream) const {
	Path::print(stream);
	stream << "\ndirPath:'" << _dirPath << "' | type: " << typeLabels[_type]
		   << " | file: '" << _file << "' | cgiExt: '" << _sufix << "'\n";
}

ostream &operator<<(ostream &os, const RequestPath &path) {
	path.print(os);
	return os;
}
