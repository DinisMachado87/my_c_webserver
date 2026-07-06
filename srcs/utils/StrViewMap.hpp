#ifndef STRVIEWMAP_HPP
#define STRVIEWMAP_HPP

#include "StrView.hpp"
#include <utility>
#include <vector>

/* Flat vector of key-value StrView pairs — O(n) linear scan.
 * Suited for small collections like HTTP headers where
 * insertion order matters and n is small. */
class StrViewMap
{
public:
	StrViewMap();
	StrViewMap(const StrViewMap &other);
	~StrViewMap();

	StrViewMap &operator=(const StrViewMap &other);

	enum e_case { SENSITIVE, INSENSITIVE };

	void print(std::ostream &stream) const;
	size_t size() const;
	const std::pair<StrView, StrView> &at(size_t i) const;

	const StrView *find(const StrView &key,
						e_case sensitivity = SENSITIVE) const;
	void set(const StrView &key, const StrView &value,
			 e_case sensitivity = INSENSITIVE);
	void setIfMissing(const StrView &key, const StrView &value,
					  e_case sensitivity = INSENSITIVE);
	void insert(std::pair<StrView, StrView> newPair);
	void insert(const StrView &key, const StrView &value);

private:
	std::vector<std::pair<StrView, StrView> > _pairs;

	static bool match(const StrView &a, const StrView &b, e_case sensitivity);
};

std::ostream &operator<<(std::ostream &os, const StrViewMap &map);

#endif
