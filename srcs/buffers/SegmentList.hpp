#pragma once

#include "BufferManager.hpp"
#include "StrView.hpp"
#include <cstddef>

/* Doubly-linked list of Segments. Tracks cumulative totalLen.
 * Segments are accessed only by popping — never expose a raw Segment*
 * to callers, to prevent cross-linking between lists.
 *
 * NOT INSTANTIABLE DIRECTLY
 * use:
 * - StreamBuffer for FIFO consumption (see IOBuffer for fd read/write)
 * - StackBuffer for LIFO consumption */
class SegmentList
{
public:
	// Returns all segments to the pool on destruction.
	virtual ~SegmentList();

	size_t totalLen() const;
	bool empty() const;

	// Appends seg to the write end. seg->_next must be NULL.
	void pushBack(Segment *seg);
	// Raw pointer — no strlen, for non-terminated buffers.
	void append(const char *data, size_t len);
	// String literals implicitly convert to StrView (calls strlen).
	void append(const StrView &sv);

private:
	// Explicit disables
	SegmentList &operator=(const SegmentList &other);
	SegmentList(const SegmentList &other);

protected:
	// disable direct instantiation - only called by inheritance
	SegmentList(BufferManager &buffManager);

	BufferManager &_buffManager;
	Segment *_readEnd;
	Segment *_writeEnd;
	size_t _totalLen;

	// Unlinks seg from the list and returns it.
	// Caller is responsible for returning it to BufferManager.
	Segment *pop(Segment *seg);
};

inline SegmentList &operator<<(SegmentList &out, const StrView &sv)
{
	out.append(sv);
	return out;
};
