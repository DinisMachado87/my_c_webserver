#pragma once

#include "PathConsolidator.hpp"
#include "RequestPath.hpp"

class RequestPathConsolidator : public PathConsolidator {
	bool _isDir;
	bool _isCgi;
	StrView _cgiExtension;

	RequestPathConsolidator(const StrView &path);
	void extractHttpInfo();

public:
	static RequestPath consolidate(const StrView &path);
};
