#include "RequestPathConsolidator.hpp"
#include "RequestPath.hpp"
#include <iostream>

RequestPathConsolidator::RequestPathConsolidator(const StrView &path) :
	PathConsolidator(path),
	_type(RequestPath::NONE) {}

void RequestPathConsolidator::extractHttpInfo() {
	if (_segments.empty())
		return;
	if ('/' == *_segments.back().getEnd()) {
		_type = RequestPath::DIR;
		if (_segments.size() > 1) {
			_segments.pop_back();
			if (_writeIdx > _segments.size())
				_writeIdx = _segments.size();
		}
	} else {
		_file = _segments.back();
		_sufix = _file.lastSplitBefore('.');
		_type = ('.' == *_sufix.getStart()) ? RequestPath::EXECUTABLE
											: RequestPath::FILE;
	}
}

void RequestPathConsolidator::trimPath() {
	if (_type == RequestPath::DIR)
		_dirPath = _path;
	else {
		_dirPath = _path;
		_dirPath.trimEnd(_file.getLen());
	}
}

RequestPath RequestPathConsolidator::consolidate(const StrView &pathStr) {
	RequestPathConsolidator c(pathStr);

	c.isSingleSlash();
	c.split();
	c.normalize();
	c.extractHttpInfo();
	c.rebuild();
	c.trimPath();
	return RequestPath(c._path, c._query, c._fragment, c._type, c._dirPath,
					   c._sufix);
}
