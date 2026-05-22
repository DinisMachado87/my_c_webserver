#ifndef PATHCONSOLIDATOR_HPP
#define PATHCONSOLIDATOR_HPP

#include "Path.hpp"
#include "StrView.hpp"
#include <vector>

class PathConsolidator {
protected:
	std::vector<StrView> _segments;
	size_t _writeIdx;
	bool _hasChanges;

	StrView _path;
	StrView _query;
	StrView _fragment;

	PathConsolidator(const StrView &path);

	bool isSingleSlash();
	void split();
	void normalize();
	void rebuild();
	void printSegments(size_t i, size_t writeIdx, size_t deletedSegs);

public:
	static Path consolidate(const StrView &path);
};

#endif
