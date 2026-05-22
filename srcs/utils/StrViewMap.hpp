#ifndef STRVIEWMAP_HPP
#define STRVIEWMAP_HPP

#include "StrView.hpp"
#include <utility>
#include <vector>
class StrViewMap {
private:
	std::vector<std::pair<StrView, StrView> > _pairs;

public:
	// Constructors and destructors
	StrViewMap();
	StrViewMap(const StrViewMap &other);
	~StrViewMap();

	// Operators overload
	StrViewMap &operator=(const StrViewMap &other);

	// Getters and setters

	// Methods
	void print(std::ostream &stream) const;
	size_t size();
	const StrView *find(StrView &key) const;
	void insert(std::pair<StrView, StrView> newPair);
};

std::ostream &operator<<(std::ostream &os, const StrViewMap &map);

#endif
