#ifndef STRVIEWMAP_HPP
#define STRVIEWMAP_HPP

#include "StrView.hpp"
#include <utility>
#include <vector>

/* Flat vector of key-value StrView pairs — O(n) linear scan.
 * Keeps the elements ordered moving operation cost from serching to storing.
 * good for cases where stored once and searched many times */
class StrViewMap
{
public:
	StrViewMap();
	StrViewMap(const StrViewMap &other);
	~StrViewMap();

	/* Operators */
	StrViewMap &operator=(const StrViewMap &other);

	void print(std::ostream &stream) const;
	size_t size();
	// Returns pointer to value, or NULL if not found.
	const StrView *find(StrView &key) const;
	// Allows duplicates.
	void insert(std::pair<StrView, StrView> newPair);

private:
	std::vector<std::pair<StrView, StrView> > _pairs;
};

std::ostream &operator<<(std::ostream &os, const StrViewMap &map);

#endif
