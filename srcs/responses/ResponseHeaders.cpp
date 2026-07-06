#include "ResponseHeaders.hpp"
#include <cstdio>
#include <cstring>

ResponseHeaders::ResponseHeaders() :
	_scratchUsed(0)
{
}
ResponseHeaders::~ResponseHeaders() {}

StrView ResponseHeaders::toScratch(const char *str, size_t len)
{
	if (_scratchUsed + len > sizeof(_scratch))
		return StrView();
	char *dest = _scratch + _scratchUsed;
	std::memcpy(dest, str, len);
	_scratchUsed += len;
	return StrView(dest, len);
}

void ResponseHeaders::set(const StrView &key, size_t numValue)
{
	char tmp[32];
	int len = std::sprintf(tmp, "%lu", static_cast<ulong>(numValue));
	StrViewMap::set(key, toScratch(tmp, static_cast<size_t>(len)));
}

void ResponseHeaders::setIfMissing(const StrView &key, size_t numValue)
{
	char tmp[32];
	int len = std::sprintf(tmp, "%lu", static_cast<ulong>(numValue));
	StrViewMap::setIfMissing(key, toScratch(tmp, static_cast<size_t>(len)));
}

void ResponseHeaders::serialize(StreamBuffer &out) const
{
	for (size_t i = 0; i < size(); i++) {
		const std::pair<StrView, StrView> &p = at(i);
		out << p.first << ": " << p.second << "\r\n";
	}
	out << "\r\n";
}
