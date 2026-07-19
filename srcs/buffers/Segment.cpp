#include "Segment.hpp"
#include "Reader.hpp"
#include "Writer.hpp"
#include <cassert>
#include <cstring>

/* Constructors */
Segment::Segment() :
	_written(0),
	_sent(0),
	_prev(NULL),
	_next(NULL)
{
}

/* Link primitives */
// Splice node in after this. node's old links are overwritten, not read.
void Segment::linkNext(Segment *node)
{
	assert(node);
	node->_prev = this;
	node->_next = _next;
	if (_next)
		_next->_prev = node;
	_next = node;
}

// Splice node in before this.
void Segment::linkPrev(Segment *node)
{
	assert(node);
	node->_next = this;
	node->_prev = _prev;
	if (_prev)
		_prev->_next = node;
	_prev = node;
}

// Detach self, patch both neighbors, null own links.
void Segment::unlink()
{
	if (_prev)
		_prev->_next = _next;
	if (_next)
		_next->_prev = _prev;
	_prev = NULL;
	_next = NULL;
}

// Poisons stale data so a use-after-return reads zeros, not old bytes.
void Segment::poison() { std::memset(_data, 0, RECV_SIZE); }
void Segment::clearCursors() { _written = 0; _sent = 0; }

/* Methods */
void Segment::reset()
{
	_prev = NULL;
	_next = NULL;
	_written = 0;
	_sent = 0;
}

ssize_t Segment::readFrom(const Reader &reader)
{
	if (writable() == 0)
		return 0;
	ssize_t n = reader.fill(_data + _written, writable());
	if (n > 0)
		_written += static_cast<size_t>(n);
	return n;
}

size_t Segment::copyIn(const char *src, size_t len)
{
	size_t space = writable();
	if (len > space)
		len = space;
	std::memcpy(_data + _written, src, len);
	_written += len;
	return len;
}

ssize_t Segment::sendTo(const Writer &writer)
{
	if (readable() == 0)
		return 0;
	ssize_t n = writer.writeOne(_data + _sent, readable());
	if (n > 0)
		_sent += static_cast<size_t>(n);
	return n;
}

/* View getters */

StrView Segment::unsentView() const
{
	return StrView(_data + _sent, readable());
}

StrView Segment::writtenView() const { return StrView(_data, used()); }

StrView Segment::lastWritten(size_t n) const
{
	assert(n <= _written);
	return StrView(_data + _written - n, n);
}

/* Capacity Getters */

bool Segment::allSent() const { return _sent >= _written; }
size_t Segment::readable() const { return _written - _sent; }
size_t Segment::writable() const { return RECV_SIZE - _written; }
size_t Segment::used() const { return _written; }

