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

void Segment::poison()
{
#ifdef DEBUG
	std::memset(_data, 0xDE, RECV_SIZE);
#else
	std::memset(_data, 0, RECV_SIZE);
#endif
}

ssize_t Segment::readFrom(ReadFunc fn, int fd)
{
	assert(fn && "readFrom: NULL read function");

	ssize_t n = fn(fd, _data + _usedLen, RECV_SIZE - _usedLen);
	if (n > 0)
		_usedLen += n;
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
		_sent += n;
	return n;
}

const char *Segment::data() const { return _data; }
size_t Segment::readable() const { return _usedLen; }
size_t Segment::writable() const { return RECV_SIZE - _usedLen; }
bool Segment::allSent() const { return _sent >= _usedLen; }
