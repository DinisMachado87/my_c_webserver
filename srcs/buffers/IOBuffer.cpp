#include "IOBuffer.hpp"
#include "BufferManager.hpp"
#include "Segment.hpp"
#include "Writer.hpp"

IOBuffer::IOBuffer(const Reader &reader, const Writer &writer,
				   BufferManager &pool) :
	IBuffer(reader, pool),
	_writer(writer),
	_curSegment(NULL)
{
}

// Pool, not recycle stack, for memset.
IOBuffer::~IOBuffer()
{
	if (_curSegment)
		_curSegment = _segPool.returnSegment(_curSegment);
}

/* Methods */
ssize_t IOBuffer::writeOut()
{
	if (!takeSegment())
		return 0;

	ssize_t bytesSent = _curSegment->sendTo(_writer);
	releaseSegment();
	return bytesSent;
}

/* Protected Methods */
bool IOBuffer::takeSegment()
{
	if (_curSegment)
		return true;

	_curSegment = _segList.popHead();
	return _curSegment != NULL;
}

// Recycle stack, not pool: reuse without memset.
void IOBuffer::releaseSegment()
{
	if (!_curSegment || !segmentSpent())
		return;

	_segList.pushToRecycleStack(_curSegment);
	_curSegment = NULL;
}

bool IOBuffer::segmentSpent() const { return _curSegment->readable() == 0; }

Segment::e_comparison IOBuffer::compareUnsent(const StrView &expected) const
{
	if (!_curSegment)
		return _segList.compare(expected);

	StrView unsent = _curSegment->unsentView();
	if (!expected.compare(unsent, unsent.size()))
		return Segment::MISMATCH;

	StrView rest = expected;
	rest.removePrefix(unsent.size());
	return _segList.compare(rest);
}
