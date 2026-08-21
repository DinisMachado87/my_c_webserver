#pragma once

#include "StrViewMap.hpp"
#include <cstddef>

class IBuffer;

/* StrViewMap with a scratch buffer for computed values (Content-Length etc.)
 * and a serializer that writes "Key: Value\r\n" pairs into a IBuffer.
 * Does NOT write the status line — caller handles that. */
class ResponseHeaders : public StrViewMap
{
private:
	char _scratch[256];
	size_t _scratchUsed;

	/* Explicit disables */
	ResponseHeaders(const ResponseHeaders &other);
	ResponseHeaders &operator=(const ResponseHeaders &other);

	StrView toScratch(const char *str, size_t len);

public:
	ResponseHeaders();
	~ResponseHeaders();

	// Added set() numeric overloads — writen as char* in _scratch buffer.
	// Using needed to bring back base class methods
	// normally hidden when added overload in inherited class
	using StrViewMap::set;
	using StrViewMap::setIfMissing;
	void set(const StrView &key, size_t numValue);
	void setIfMissing(const StrView &key, size_t numValue);

	// Writes "Key: Value\r\n" for each header + final "\r\n".
	void serialize(IBuffer &out) const;
};
