#include "StrView.hpp"
#include "Colors.hpp"
#include "Logger.hpp"
#include "webServ.hpp"
#include <climits>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using std::ostream;
using std::runtime_error;
using std::string;
using std::stringstream;
using std::vector;

string StrView::_empty = "";

// Public constructors and destructors
StrView::StrView(std::string &buffer, const uint offset, const uint len) :
	_rawBuffer(&buffer),
	_offset(offset),
	_len(len) {}

StrView::StrView(std::string *buffer, const uint offset, const uint len) :
	_rawBuffer(&(*buffer)),
	_offset(offset),
	_len(len) {}

StrView::StrView() :
	_rawBuffer(&_empty),
	_offset(0),
	_len(0) {}

StrView::StrView(std::string &buffer) :
	_rawBuffer(&buffer),
	_offset(0),
	_len(0) {}

StrView::StrView(const StrView &other) :
	_rawBuffer(other._rawBuffer),
	_offset(other._offset),
	_len(other._len) {}

StrView::~StrView() {}

// Operators overload
StrView &StrView::operator=(const StrView &other) {
	if (this == &other)
		return *this;

	this->_rawBuffer = other._rawBuffer;
	this->_offset = other._offset;
	this->_len = other._len;

	return *this;
}

StrView &StrView::operator=(StrView &other) {
	if (this == &other)
		return *this;

	this->_rawBuffer = other._rawBuffer;
	this->_offset = other._offset;
	this->_len = other._len;

	return *this;
}

bool StrView::operator==(const StrView &other) const {
	if (_len != other._len)
		return false;
	return strncmp(getStart(), other.getStart(), _len) == 0;
}

bool StrView::operator!=(const StrView &other) const {
	return !(*this == other);
}

bool StrView::operator==(const char *str) const { return compare(str); }
bool StrView::operator!=(const char *str) const { return !(*this == str); }

bool StrView::operator<(const StrView &other) const {
	int cmpResult = strncmp(getStart(), other.getStart(),
							_len < other._len ? _len : other._len);
	if (cmpResult != 0)
		return cmpResult < 0;
	return _len < other._len;
}

// Getters
const char *StrView::getStart() const { return _rawBuffer->c_str() + _offset; };
const char *StrView::getEnd() const { return getStart() + (_len - 1); }
string StrView::getStr() const { return string(getStart(), _len); }
size_t StrView::getBufferSize() const { return _rawBuffer->length(); }
uint StrView::getOffset() const { return _offset; };
uint StrView::getLen() const { return _len; };

// Setters
void StrView::setBuffer(std::string &newBuffer) { _rawBuffer = &newBuffer; }
void StrView::setLen(uint len) { _len = len; }

void StrView::setStart(const char *start) {
	_offset = start - _rawBuffer->c_str();
}

void StrView::setStartAndLen(const char *start, uint len) {
	_offset = start - _rawBuffer->c_str();
	_len = len;
}

// Public Methods
void StrView::updateOffset(uint increase) { _offset += increase; }
void StrView::printStrV() const { write(1, getStart(), _len); }
void StrView::streamStrV(std::stringstream &stream) const {
	stream << getStr() << "\n";
}
void StrView::printBuffer() const {
	write(1, _rawBuffer->c_str(), _rawBuffer->length());
}
bool StrView::compare(StrView &strV) const { return compare(strV.getStart()); }
void StrView::nullTerminate() { _rawBuffer[_offset + _len - 1] = '\0'; }
void StrView::trimEnd(const size_t trimSize) {
	_len > trimSize ? _len -= trimSize : 0;
}

bool StrView::compare(const char *str) const {
	if (OK == strncmp(getStart(), str, _len) && str[_len] == '\0')
		return true;
	return false;
};

bool StrView::ncompare(const char *str, size_t len) const {
	if (OK == strncmp(getStart(), str, len))
		return true;
	return false;
};

void StrView::move(std::string &toBuffer) {
	LOG(Logger::CONTENT, getStr().c_str());

	int offset = toBuffer.length();
	toBuffer.append(getStart(), _len);
	toBuffer.push_back('\0');
	_rawBuffer = &toBuffer;
	_offset = offset;
}

void StrView::replace(const uint startOffset, const StrView &toInsert) {
	uint insertStart = _offset + startOffset;
	_rawBuffer->replace(insertStart, toInsert.getLen(), toInsert.getStart());
}

void StrView::nreplace(const uint startOffset, const StrView &toInsert,
					   const uint len) {
	uint insertStart = _offset + startOffset;
	_rawBuffer->replace(insertStart, len, toInsert.getStr());
}

const char *StrView::has(const char *color, const bool hasColor) const {
	return (hasColor ? color : "");
}

void StrView::info(ostream &ostream, const char item, const char *color) const {
	const bool s = (item == 's');				   // [s]tring
	const bool p = (item == 'a') || (item == 'p'); // [a]ll || buffer [p]ointer
	const bool l = (item == 'a') || (item == 'l'); // [a]ll || [l]ength
	const bool o = (item == 'a') || (item == 'o'); // [a]ll || [o]ffset

	ostream << (s ? color : YELLOW) << getStr() << RESET << " |";

	ostream << has(color, p) << " pt:" << std::hex
			<< (reinterpret_cast<size_t>(_rawBuffer) & 0xFF) << std::dec
			<< has(RESET, p);

	ostream << has(color, l) << " l:" << _len << has(RESET, l);
	ostream << has(color, o) << " o:" << _offset << has(RESET, o);
}

void StrView::streamStrView(stringstream &ostream) {
	ostream << getStr();
	ostream << "|buf:" << &_rawBuffer << "|len:" << _len << "|off:" << _offset;
}

void StrView::intoStream(ostream &stream) const {
	const char *str = getStart();
	str ? stream.write(str, getLen()) : stream << "NULL";
}

size_t StrView::findPosInBuffer(const char c, const size_t addedOffset) const {
	return _rawBuffer->find(c, _offset + addedOffset);
}

size_t StrView::find(const char c, const size_t addedOffset) const {
	if (_len <= addedOffset)
		return string::npos;

	size_t posInBuff = findPosInBuffer(c, addedOffset);
	size_t posInStrView;

	if (posInBuff == string::npos || posInBuff >= (_offset + _len))
		return string::npos;

	posInStrView = posInBuff - _offset;
	if (posInStrView > UINT_MAX)
		throw runtime_error(
			TRACED("uint overflow. StrView uses uint for memory lightness"));

	return posInStrView;
}

/*
 * Extracts a segment from startOffset until the next separator.
 * Returns the position of the separator, or string::npos if end reached.
 *
 * Ex. "/path/to" from offset 0 with '/'
 *     -> segment = "/path", returns 5
 */
size_t StrView::segmentUntil(char separator, uint startOffset,
							 StrView &segment) const {
	if (startOffset >= _len) {
		segment = StrView(_rawBuffer, _offset + _len, 0);
		return string::npos;
	}

	size_t size_tNextPos = find(separator, startOffset + 1);
	const bool isLast = (size_tNextPos == string::npos);

	uint newoffset = _offset + startOffset;
	uint nextPos = static_cast<uint>(size_tNextPos);
	uint segLen;

	if (!isLast && size_tNextPos > UINT_MAX)
		throw runtime_error(TRACED("uint overflow"));

	segLen = (isLast) ? _len - startOffset : nextPos - startOffset;

	segment = StrView(_rawBuffer, newoffset, segLen);
	return size_tNextPos;
}

StrView StrView::lastSplitBefore(const char c) const {
	size_t curOffset = 0;
	while (1) {
		size_t nextDivider = find(c, curOffset);
		if (nextDivider == string::npos)
			return StrView(_rawBuffer, curOffset, nextDivider);
		if (nextDivider > UINT_MAX)
			throw runtime_error(TRACED("uint overflow"));
		curOffset = static_cast<uint>(nextDivider);
	}
}

vector<StrView> StrView::splitBefore(const char c) const {
	vector<StrView> splitVec;
	StrView cur = *this;
	uint curOffset = 0;

	while (1) {
		size_t nextDivider = segmentUntil(c, curOffset, cur);
		if (nextDivider > UINT_MAX)
			throw runtime_error(TRACED("uint overflow"));

		splitVec.push_back(cur);
		if (nextDivider == string::npos)
			break;
		curOffset = static_cast<uint>(nextDivider);
	}
	return splitVec;
}
