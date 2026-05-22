#ifndef PATH_HPP
#define PATH_HPP

#include "StrView.hpp"
#include <ostream>

class Path {
private:
	StrView _path;
	StrView _query;
	StrView _fragment;

protected:
	friend class PathConsolidator;
	friend class RequestPath;
	friend class PathTestBase;
	friend class PathTest;

	// Constructors and destructors
	Path();
	Path(const StrView &path);
	Path(const StrView &path, const StrView &query, const StrView &fragment);

public:
	// Getters
	StrView const &getPath() const;
	StrView const &getQuery() const;
	StrView const &getFragment() const;

	// Debug
	virtual void print(std::ostream &stream) const;
};

std::ostream &operator<<(std::ostream &os, const Path &path);

#endif
