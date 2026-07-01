#include "Path.hpp"
#include "StrView.hpp"
#include <iostream>

using std::ostream;

// Constructors and destructors
Path::Path() {};

Path::Path(const StrView &path) :
	_path(path) {}

Path::Path(const StrView &path, const StrView &query, const StrView &fragment) :
	_path(path),
	_query(query),
	_fragment(fragment) {}

// Getters
StrView const &Path::path() const { return _path; }
StrView const &Path::getQuery() const { return _query; }
StrView const &Path::getFragment() const { return _fragment; }

// Debug
void Path::print(ostream &stream) const {
	stream << "Path:\t'" << _path << "' | query: '" << _query << "' | frag: '"
		   << _fragment << "'";
}

ostream &operator<<(ostream &os, const Path &path) {
	path.print(os);
	return os;
}
