#pragma once

#include <cstring>
#include <ostream>
#include <string>
#include <vector>

class StrView
{
	const char *_data;
	size_t _size;

public:
	// Non-owning view over a contiguous buffer.
	// Caller is responsible for ensuring the buffer outlives the view.

	StrView();
	StrView(const char *str);
	StrView(const char *data, size_t size);
	StrView(char *data, size_t size);
	StrView(const std::string &s);
	StrView(const StrView &other);
	~StrView();

	// Operators
	StrView &operator=(const StrView &other);
	bool operator==(const StrView &other) const;
	bool operator!=(const StrView &other) const;
	bool operator==(const char *str) const;
	bool operator!=(const char *str) const;
	bool operator<(const StrView &other) const;

	// Getters
	const char *data() const; // pointer to first char
	const char *end() const;  // one-past-end, do not dereference
	size_t size() const;
	bool empty() const;
	std::string getStr() const; // allocates; avoid

	// Methods
	void consolidate(char *dest);
	void printBuffer() const;
	void setStart(const char *str);
	void setSize(size_t size);
	void removePrefix(size_t n);
	void removeSuffix(size_t n);

	bool compare(const StrView &other) const;
	bool compare(const StrView &other, size_t len) const;

	size_t find(char c, size_t offset = 0) const;
	size_t segmentUntil(char sep, size_t offset, StrView &out) const;

	std::vector<StrView> splitBefore(char c) const;
	StrView lastSplitBefore(char c) const;
	void intoStream(std::ostream &os) const;
};

inline std::ostream &operator<<(std::ostream &os, const StrView &sv)
{
	sv.intoStream(os);
	return os;
}
