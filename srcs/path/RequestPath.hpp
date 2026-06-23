#pragma once

#include "Path.hpp"
#include "StrView.hpp"
#include "webServ.hpp"

#define TYPE_LIST                                              \
	type(NONE) type(DIR) type(FILE) type(CGI) type(EXECUTABLE) \
		type(CGI_W_INTERPRETER)

class RequestPath : public Path {
protected:
	friend class RequestPathConsolidatorTest;
	friend class RequestPathConsolidator;
	friend class RequestLine;

	RequestPath(const StrView &path, const StrView &query,
				const StrView &fragment, const uchar type,
				const StrView &dirPath, const StrView &sufix);

	uchar _type;
	StrView _file;
	StrView _dirPath;
	StrView _sufix;

public:
	enum pathType {
#define type(value) value,
		TYPE_LIST
#undef type
			SIZE
	};

	static const char *typeLabels[SIZE];

	RequestPath();
	RequestPath(const StrView &pathStr);

	uchar getType() const;
	StrView const &getCgiExtension() const;

	void print(std::ostream &stream) const;
};

std::ostream &operator<<(std::ostream &os, const RequestPath &path);
