#include "StrViewMap.hpp"
#include "StrView.hpp"
#include <utility>

using std::pair;
using std::vector;

// Public constructors and destructors
StrViewMap::StrViewMap() {}

StrViewMap::StrViewMap(const StrViewMap &other) :
	_pairs(other._pairs) {}

StrViewMap::~StrViewMap() {}

StrViewMap &StrViewMap::operator=(const StrViewMap &other) {
	if (this == &other)
		return *this;
	this->_pairs = other._pairs;
	return *this;
}

// Public Methods
void StrViewMap::insert(pair<StrView, StrView> newPair) {
	_pairs.push_back(newPair);
}

const StrView *StrViewMap::find(StrView &key) const {
	vector<pair<StrView, StrView> >::const_iterator end = _pairs.end();
	vector<pair<StrView, StrView> >::const_iterator cur = _pairs.begin();
	for (; cur != end; cur++) {
		StrView curKey = cur->first;
		if (key == curKey)
			return &cur->second;
	}
	return NULL;
};

size_t StrViewMap::size() { return _pairs.size(); }

void StrViewMap::print(std::ostream &stream) const {
	vector<pair<StrView, StrView> >::const_iterator end = _pairs.end();
	vector<pair<StrView, StrView> >::const_iterator cur = _pairs.begin();
	for (; cur != end; cur++) {
		StrView curKey = cur->first;
		StrView curValue = cur->second;
		stream << curKey << ": " << curValue << "\n";
	}
}

std::ostream &operator<<(std::ostream &os, const StrViewMap &map) {
	map.print(os);
	return os;
}
