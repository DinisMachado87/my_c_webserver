#pragma once

#include <cstring>
#include <ostream>
#include <string>
#include <vector>

/* Non-owning view over a contiguous buffer.
 * Caller is responsible for ensuring the buffer outlives the view. */
class StrView
{
	const char *_data;
	size_t _size;

public:
	enum splitPosition { BEFORE, AFTER };

	StrView();
	StrView(const char *str);
	StrView(const char *data, size_t size);
	StrView(char *data, size_t size);
	StrView(const std::string &s);
	StrView(const StrView &other);
	~StrView();

	/* Operators */
	StrView &operator=(const StrView &other);
	bool operator==(const StrView &other) const;
	bool operator!=(const StrView &other) const;
	bool operator==(const char *str) const;
	bool operator!=(const char *str) const;
	bool operator<(const StrView &other) const;

	/* Getters */
	const char *data() const;
	const char *end() const;
	size_t size() const;
	bool empty() const;
	std::string getStr() const; // allocates; avoid

	/* In-place overwrites — writes through const pointer via const_cast.
	 * Only safe when the view points into writable memory. */
	void replace(const std::string &src);
	void replace(size_t offset, const StrView &src, size_t len);
	void replace(const StrView &src, size_t len);
	void replace(const StrView &src);
	// Copies data to dest and repoints the view there.
	void consolidate(char *dest);

	/* Mutators — shift the view, don't change data */
	void setStart(const char *str);
	void setSize(size_t size);
	void removePrefix(size_t n);
	void removeSuffix(size_t n);

	bool compare(const StrView &other) const;
	bool compare(const StrView &other, size_t len) const;

	/* Search */
	size_t find(char c, size_t offset = 0) const;
	// Extracts substring from offset to next sep. Returns position of sep, or
	// npos.
	size_t segmentUntil(char sep, size_t offset, StrView &out) const;
	// Splits keeping separator at the start of each segment.
	std::vector<StrView> splitBefore(char c) const;
	// Returns tail after last occurrence of c.
	StrView lastSplit(char c, splitPosition trimPosition = AFTER) const;

	void printBuffer() const;
	void intoStream(std::ostream &os) const;
};

inline std::ostream &operator<<(std::ostream &os, const StrView &sv)
{
	sv.intoStream(os);
	return os;
}

// Concatenates into a new std::string. Allocates.
inline std::string operator+(const StrView &a, const StrView &b)
{
	std::string result;
	result.reserve(a.size() + b.size());
	result.append(a.data(), a.size());
	result.append(b.data(), b.size());
	return result;
}
