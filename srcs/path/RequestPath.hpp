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
				const StrView &fragment, const uchar type,
				const StrView &dirPath, const StrView &sufix);

	uchar _type;
	StrView _file;
	StrView _dirPath;
	StrView _sufix;

public:
	enum pathType { NONE, DIR, FILE, CGI, EXECUTABLE, CGI_W_INTERPRETER, SIZE };
	static const char *typeLabels[SIZE];

	uchar getType() const;
	StrView const &getCgiExtension() const;

	void print(std::ostream &stream) const;
};

std::ostream &operator<<(std::ostream &os, const RequestPath &path);
