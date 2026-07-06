#include "StrViewMap.hpp"
#include "StrView.hpp"
#include <cctype>
#include <utility>

using std::pair;
using std::vector;

StrViewMap::StrViewMap() {}
StrViewMap::StrViewMap(const StrViewMap &other) :
	_pairs(other._pairs)
{
}

StrViewMap::~StrViewMap() {}

StrViewMap &StrViewMap::operator=(const StrViewMap &other)
{
	if (this == &other)
		return *this;
	_pairs = other._pairs;
	return *this;
}

bool StrViewMap::match(const StrView &a, const StrView &b, e_case sensitivity)
{
	if (a.size() != b.size())
		return false;
	if (SENSITIVE == sensitivity)
		return a == b;
	for (size_t i = 0; i < a.size(); i++)
		if (std::tolower(a.data()[i]) != std::tolower(b.data()[i]))
			return false;
	return true;
}

size_t StrViewMap::size() const { return _pairs.size(); }

const pair<StrView, StrView> &StrViewMap::at(size_t i) const
{
	return _pairs[i];
}

const StrView *StrViewMap::find(const StrView &key, e_case sensitivity) const
{
	vector<pair<StrView, StrView> >::const_iterator cur = _pairs.begin();
	for (; cur != _pairs.end(); cur++)
		if (match(key, cur->first, sensitivity))
			return &cur->second;
	return NULL;
}

void StrViewMap::insert(pair<StrView, StrView> newPair)
{
	_pairs.push_back(newPair);
}

void StrViewMap::insert(const StrView &key, const StrView &value)
{
	_pairs.push_back(std::make_pair(key, value));
}

void StrViewMap::set(const StrView &key, const StrView &value,
					 e_case sensitivity)
{
	vector<pair<StrView, StrView> >::iterator cur = _pairs.begin();
	for (; cur != _pairs.end(); cur++) {
		if (match(key, cur->first, sensitivity)) {
			cur->second = value;
			return;
		}
	}
	_pairs.push_back(std::make_pair(key, value));
}

void StrViewMap::setIfMissing(const StrView &key, const StrView &value,
							  e_case sensitivity)
{
	if (!find(key, sensitivity))
		_pairs.push_back(std::make_pair(key, value));
}

void StrViewMap::print(std::ostream &stream) const
{
	vector<pair<StrView, StrView> >::const_iterator cur = _pairs.begin();
	for (; cur != _pairs.end(); cur++)
		stream << cur->first << ": " << cur->second << "\n";
}

std::ostream &operator<<(std::ostream &os, const StrViewMap &map)
{
	map.print(os);
	return os;
}
