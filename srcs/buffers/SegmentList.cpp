#include "SegmentList.hpp"
#include "BufferManager.hpp"
#include "Segment.hpp"
#include <cassert>

SegmentList::SegmentList(BufferManager &segmentPool) :
	_segmentPool(segmentPool),
	_head(NULL),
	_tail(NULL),
	_recycleStack(NULL)
{
}

SegmentList::~SegmentList() { clear(); }

/* Methods */

// Walk main chain then recycle stack, return every seg to pool, null endpoints.
// Return whole _next chain to pool. Reads raw links — inside friend boundary.
void SegmentList::drain(Segment *seg)
{
	while (seg) {
		Segment *next = seg->_next;
		_segmentPool.returnSegment(seg);
		seg = next;
	}
}

void SegmentList::clear()
{
	drain(_head);
	drain(_recycleStack);
	_head = NULL;
	_tail = NULL;
	_recycleStack = NULL;
}

void SegmentList::pushTail(Segment *seg)
{
	assert(seg);
	if (_tail)
		_tail->linkNext(seg);
	else
		_head = seg;
	_tail = seg;
}

void SegmentList::pushHead(Segment *seg)
{
	assert(seg);
	if (seg->allSent()) {
		pushStack(seg);
		return;
	}
	if (_head)
		_head->linkPrev(seg);
	else
		_tail = seg;
	_head = seg;
}

Segment *SegmentList::popHead()
{
	if (!_head)
		return NULL;

	Segment *returnSeg = _head;
	_head = _head->_next;
	if (!_head)
		_tail = NULL;

	returnSeg->unlink();
	return returnSeg;
}

Segment *SegmentList::popTail()
{
	if (!_tail)
		return NULL;

	Segment *returnSeg = _tail;
	_tail = _tail->_prev;
	if (!_tail)
		_head = NULL;

	returnSeg->unlink();
	return returnSeg;
}

/* Recycle stack — singly-linked via _next, LIFO */

// Push seg onto recycle stack, cursors zeroed. LIFO via _next, no _prev.
void SegmentList::pushStack(Segment *seg)
{
	assert(seg);
	seg->clearCursors();
	seg->_next = _recycleStack;
	_recycleStack = seg;
}

Segment *SegmentList::popStack()
{
	Segment *seg = _recycleStack;
	if (!seg)
		return NULL;
	_recycleStack = seg->_next;
	seg->unlink();
	return seg;
}

// Move all Segments from list to stack with cursors reset.
void SegmentList::reset()
{
	Segment *seg = _head;
	while (seg) {
		Segment *next = seg->_next;
		pushStack(seg);
		seg = next;
	}
	_head = NULL;
	_tail = NULL;
}
