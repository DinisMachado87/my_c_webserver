#include "RequestBuffer.hpp"
#include "BufferManager.hpp"
#include "Segment.hpp"

const ssize_t RequestBuffer::CARRY_OVERFLOW;

RequestBuffer::RequestBuffer(const Reader &reader, BufferManager &pool) :
	IBuffer(reader, pool),
	_active(NULL)
{
}

RequestBuffer::~RequestBuffer()
{
	if (_active)
		_segPool.returnSegment(_active);
}

ssize_t RequestBuffer::readIn()
{
	if (!_active)
		_active = _segPool.getSegment();
	else if (_active->writable() < RECV_SIZE / 3 && !moveLeftoverToNewSegment())
		return CARRY_OVERFLOW;

	ssize_t bytesRead = _active->readFrom(_reader);
	if (bytesRead == 0) {
		_inClosed = true;
		onEof();
	}
	return bytesRead;
}

StrView RequestBuffer::unparsed() const { return _active->unusedView(); }

void RequestBuffer::consumed(size_t n) { _active->advanceUsed(n); }

// Retain the full segment (parsed StrViews reference it), move the
// unconsumed tail to a fresh segment's front. Single copy.
bool RequestBuffer::moveLeftoverToNewSegment()
{
	StrView leftover = _active->unusedView();
	if (leftover.size() == RECV_SIZE)
		return false; // invalid token bigger than segment size

	_segList.pushTail(_active);
	_active = _segPool.getSegment();
	if (!leftover.empty())
		_active->copyIn(leftover.data(), leftover.size());
	return true;
}
