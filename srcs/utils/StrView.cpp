#include "StrView.hpp"
#include "Logger.hpp"
#include <climits>
#include <cstring>
#include <string>
#include <unistd.h>
#include <vector>

using std::string;
using std::vector;

// _data is never NULL; default and null-input cases point here instead.
// This makes strncmp/memchr safe to call with size 0 without extra guards.
static const char s_empty[] = "";

// Constructors
StrView::StrView() :
	_data(s_empty),
	_size(0) {}

StrView::StrView(const char *str) :
	_data(str ? str : s_empty),
	_size(str ? std::strlen(str) : 0) {}

StrView::StrView(const char *data, size_t size) :
	_data(data ? data : s_empty),
	_size(data ? size : 0) {}

StrView::StrView(char *data, size_t size) :
	_data(data ? data : s_empty),
	_size(data ? size : 0) {}

StrView::StrView(const std::string &s) :
	_data(s.c_str()),
	_size(s.size()) {}

StrView::StrView(const StrView &other) :
	_data(other._data),
	_size(other._size) {}

StrView::~StrView() {}

// Operators
StrView &StrView::operator=(const StrView &other) {
	if (this == &other)
		return *this;
	_data = other._data;
	_size = other._size;
	return *this;
}

bool StrView::operator==(const StrView &other) const { return compare(other); }

bool StrView::operator!=(const StrView &other) const {
	return !(*this == other);
}

bool StrView::operator==(const char *str) const {
	return *this == StrView(str);
}

bool StrView::operator!=(const char *str) const { return !(*this == str); }

bool StrView::operator<(const StrView &other) const {
	int r = std::strncmp(_data, other._data,
						 _size < other._size ? _size : other._size);
	if (r != 0)
		return r < 0;
	return _size < other._size;
}

// Getters
const char *StrView::data() const { return _data; }
const char *StrView::end() const { return _data + _size; }
size_t StrView::size() const { return _size; }
bool StrView::empty() const { return _size == 0; }
string StrView::getStr() const { return string(_data, _size); }

void StrView::setSize(size_t size) { _size = size; }
void StrView::setStart(const char *str) { _data = str; }

// Modifiers
void StrView::removePrefix(size_t n) {
	if (n >= _size) {
		_data += _size;
		_size = 0;
	} else {
		_data += n;
		_size -= n;
	}
}

void StrView::removeSuffix(size_t n) {
	if (n >= _size)
		_size = 0;
	else
		_size -= n;
}

// Methods
bool StrView::compare(const StrView &other) const {
	if (_size != other._size)
		return false;
	return compare(other, _size);
}

bool StrView::compare(const StrView &other, size_t len) const {
	if (len > _size || len > other._size)
		return false;
	return std::strncmp(_data, other._data, len) == 0;
}

size_t StrView::find(char c, size_t offset) const {
	if (offset >= _size)
		return string::npos;
	const char *p = static_cast<const char *>(
		std::memchr(_data + offset, c, _size - offset));
	if (!p)
		return string::npos;
	return static_cast<size_t>(p - _data);
}

/*
 * Extracts segment from offset until next occurrence of sep.
 * out is set to the segment. Returns position of sep or npos if end reached.
 *
 * Ex. "/path/to" from offset 0 with '/'
 *     -> out = "/path", returns 5
 */
size_t StrView::segmentUntil(char sep, size_t offset, StrView &out) const {
	if (offset >= _size) {
		out = StrView(_data + _size, 0);
		return string::npos;
	}
	size_t next = find(sep, offset + 1);
	size_t segLen = (next == string::npos) ? _size - offset : next - offset;
	out = StrView(_data + offset, segLen);
	return next;
}

vector<StrView> StrView::splitBefore(char c) const {
	vector<StrView> result;
	size_t offset = 0;
	StrView seg;

	while (1) {
		size_t next = segmentUntil(c, offset, seg);
		result.push_back(seg);
		if (next == string::npos)
			break;
		offset = next;
	}
	return result;
}

StrView StrView::lastSplitBefore(char c) const {
	size_t offset = 0;
	while (1) {
		size_t next = find(c, offset);
		if (next == string::npos)
			return StrView(_data + offset, _size - offset);
		offset = next + 1;
	}
}

void StrView::printBuffer() const { write(1, _data, _size); }

void StrView::intoStream(std::ostream &os) const {
	if (_size)
		os.write(_data, _size);
}

void StrView::move(char *dest) {
	LOG(Logger::CONTENT, getStr().c_str());

	memmove(dest, _data, _size);
	_data = dest;
}
