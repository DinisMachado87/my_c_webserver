#include "Segment.hpp"
#include <cassert>
#include <cstring>

Segment::Segment() :
	_prev(NULL),
	_next(NULL),
	_usedLen(0),
	_sent(0)
{
}

void Segment::reset()
{
	_prev = NULL;
	_next = NULL;
	_usedLen = 0;
	_sent = 0;
}

// Nulls buffer content — poisons stale data so a use-after-return
// reads zeros, not old bytes.
void Segment::poison() { std::memset(_data, 0, RECV_SIZE); }

ssize_t Segment::readFrom(ReadFunc fn, int fd)
{
	assert(fn && "readFrom: NULL read function");
	ssize_t n = fn(fd, _data + _usedLen, RECV_SIZE - _usedLen);
	if (n > 0)
		_usedLen += static_cast<size_t>(n);
	return n;
}

size_t Segment::copyIn(const char *src, size_t len)
{
	size_t space = RECV_SIZE - _usedLen;
	if (len > space)
		len = space;
	std::memcpy(_data + _usedLen, src, len);
	_usedLen += len;
	return len;
}

ssize_t Segment::sendTo(WriteFunc fn, int fd)
{
	assert(fn && "sendTo: NULL write function");
	ssize_t n = fn(fd, _data + _sent, _usedLen - _sent);
	if (n > 0)
		_sent += static_cast<size_t>(n);
	return n;
}

bool Segment::allSent() const { return _sent >= _usedLen; }

const char *Segment::data() const { return _data; }

size_t Segment::readable() const { return _usedLen - _sent; }
size_t Segment::writable() const { return RECV_SIZE - _usedLen; }
size_t Segment::used() const { return _usedLen; }
