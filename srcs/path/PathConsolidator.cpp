#include "PathConsolidator.hpp"
#include "Colors.hpp"
#include "Logger.hpp"
#include "Path.hpp"
#include <climits>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using std::runtime_error;
using std::string;

PathConsolidator::PathConsolidator(const StrView &pathStr) :
	_writeIdx(0),
	_hasChanges(false),
	_path(pathStr) {}

// Static
Path PathConsolidator::consolidate(const StrView &pathStr) {
	PathConsolidator c(pathStr);

	c.isSingleSlash();
	c.split();
	c.normalize();
	c.rebuild();

	return Path(c._path, c._query, c._fragment);
}

bool PathConsolidator::isSingleSlash() {
	if (_path.size() == 2 && _path.compare("/.", 2))
		_path.setSize(1);
	if (_path.size() <= 1)
		return true;
	return false;
}

void PathConsolidator::split() {
	uint curOffset = 0;
	StrView cur = _path;
	size_t nextDivider = _path.segmentUntil('/', curOffset, cur);

	while (nextDivider != string::npos) {
		if (nextDivider > UINT_MAX)
			throw runtime_error(TRACED("uint overflow"));

		if (cur.compare("/") || cur.compare("/.")) {
			const bool isLastSegment = _path.size() - 1 == nextDivider;
			if (isLastSegment) {
				cur.setSize(1); // if '/.' truncates to '/'
				_segments.push_back(cur);
				return;
			}
		} else
			_segments.push_back(cur);
		curOffset = static_cast<uint>(nextDivider);
		nextDivider = _path.segmentUntil('/', curOffset, cur);
	}

	if (cur.size() > 0)
		_segments.push_back(cur);
}

void PathConsolidator::normalize() {
	for (size_t i = 0; i < _segments.size(); i++) {
		StrView seg = _segments[i];
		bool isLastSegment = (i == _segments.size() - 1);
		bool isLastEmpty = isLastSegment && seg.compare("/.");
		if (!isLastEmpty) {
			if (seg.size() == 3 && seg.compare("/..")) {
				if (_writeIdx == 0)
					throw runtime_error("Path contains negative level");
				_writeIdx--;
			} else {
				if (_writeIdx != i)
					_segments[_writeIdx] = _segments[i];
				_writeIdx++;
			}
		}
		if (_writeIdx != i)
			_hasChanges = true;
	}
}

void PathConsolidator::rebuild() {
	if (_writeIdx == 0) {
		_path.setSize(1);
		return;
	}
	string newPathStr;
	newPathStr.reserve(_path.size());
	for (size_t i = 0; i < _writeIdx; i++)
		newPathStr.append(_segments[i].data(), _segments[i].size());
	DEBUG(cout << "\nCOPYING:\n" << newPathStr << " | temp str\n");
	_path.replace(newPathStr);
	_path.setSize(newPathStr.size());
}

void PathConsolidator::printSegments(size_t i, size_t writeIdx,
									 size_t deletedSegs) {
	size_t len = _segments.size() - deletedSegs;
	for (size_t j = 0; j < _segments.size(); j++) {
		const bool wi = (j == writeIdx);
		const bool cur = (j == i);
		const char *color = (wi ? RED : cur ? GREEN : "");
		const char *reset = ((wi || cur) ? RESET : "");

		cout << color << ((wi || cur) ? "[" : " ") << reset;
		cout << _segments[j];
		cout << color << ((wi || cur) ? "]" : " ") << reset;

		((j + 1) == len ? cout << YELLOW << " | " << RESET : cout << "   ");
	}
	cout << " |len: " << len << endl;
}
