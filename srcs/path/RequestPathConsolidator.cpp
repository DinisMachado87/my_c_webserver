#include "RequestPathConsolidator.hpp"
#include <iostream>

RequestPathConsolidator::RequestPathConsolidator(const StrView &path) :
	PathConsolidator(path),
	_isDir(false),
	_isCgi(false) {}

void RequestPathConsolidator::extractHttpInfo() {
	if (_segments.empty())
		return;
	if ('/' == *_segments.back().getEnd()) {
		_isDir = true;
	} else {
		StrView suffix = _segments.back().lastSplitBefore('.');
		std::cout << "Sufix: " << suffix << "\n";
		if ('.' == *suffix.getStart()) {
			_isCgi = true;
			_cgiExtension = suffix;
		}
	}
}

RequestPath RequestPathConsolidator::consolidate(const StrView &pathStr) {
	RequestPathConsolidator c(pathStr);

	c.isSingleSlash();
	c.split();
	c.normalize();
	c.extractHttpInfo();
	c.rebuild();
	return RequestPath(c._path, c._fragment, c._query, c._isDir, c._isCgi,
					   c._cgiExtension);
}
