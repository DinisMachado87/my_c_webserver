#pragma once

#include <cstddef>

class BufferManager;
class Segment;

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

	bool empty() const;

private:
	/* State */
	BufferManager &_segmentPool;
	Segment *_head;
	Segment *_tail;
	Segment *_recycleStack;

	/* Recycle stack Methods */
	void clear();
	void clearCursors();
	void pushStack(Segment *seg);
	Segment *popStack();

	/* Explicit disables */
	SegmentList &operator=(const SegmentList &other);
	SegmentList(const SegmentList &other);
};
