#pragma once

#include "PathConsolidator.hpp"
#include "RequestPath.hpp"
#include "StrView.hpp"

class RequestPathConsolidator : public PathConsolidator {
	uchar _type;
	StrView _sufix;
	StrView _file;
	StrView _dirPath;

	RequestPathConsolidator(const StrView &path);
	void extractHttpInfo();
	void trimPath();

public:
	static RequestPath consolidate(const StrView &path);
};
