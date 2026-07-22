#pragma once

#include "Segment.hpp"
#include <cstddef>
#include <ostream>

class BufferManager;

/* Doubly-linked list of Segments. Thin endpoint bookkeeping over
 * Segment link primitives. Borrows from a BufferManager, returns every
 * held segment in clear() — no caller drain step.
 * _recycleStack holds fully-sent segments for reuse without re-poison. */
class SegmentList
{
public:
	/* Constructors */
	explicit SegmentList(BufferManager &segmentPool);
	~SegmentList();

	/* Methods */
	void reset();
	void drain(Segment *seg);

	void pushTail(Segment *seg);
	void pushHead(Segment *seg);
	Segment *popHead();
	Segment *popTail();

	void pushToRecycleStack(Segment *seg);

	bool empty() const;
	void print(std::ostream &os) const;

	Segment::e_comparison compare(const StrView &expected) const;

private:
	/* State */
	BufferManager &_segmentPool;
	Segment *_head;
	Segment *_tail;
	Segment *_recycleStack;

	/* Recycle stack Methods */
	void clear();
	void clearCursors();
	Segment *popStack();

	/* Explicit disables */
	SegmentList &operator=(const SegmentList &other);
	SegmentList(const SegmentList &other);
};

inline std::ostream &operator<<(std::ostream &os, const SegmentList &list)
{
	list.print(os);
	return os;
}
