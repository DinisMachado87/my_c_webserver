#pragma once

#include "Path.hpp"
#include "StrView.hpp"

class RequestPath : public Path {
protected:
	friend class RequestPathConsolidatorTest;
	friend class RequestPathConsolidator;
	friend class RequestLine;

	RequestPath();
	RequestPath(const StrView &pathStr);
	RequestPath(const StrView &path, const StrView &query,
				const StrView &fragment, const bool isDir, const bool isCgi,
				const StrView &cgiExtension);

	bool _isDir;
	bool _isCgi;
	StrView _cgiExtension;

public:
	bool isDir() const;
	bool isCgi() const;
	StrView const &getCgiExtension() const;

	void print(std::ostream &stream) const;
};

std::ostream &operator<<(std::ostream &os, const RequestPath &path);
