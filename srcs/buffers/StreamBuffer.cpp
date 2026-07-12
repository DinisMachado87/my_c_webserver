#include "StreamBuffer.hpp"
#include <sys/socket.h>

StreamBuffer::StreamBuffer(BufferManager &buffManager) :
	SegmentList(buffManager)
{
}

Segment *StreamBuffer::popFront()
{
	if (!_readEnd)
		return NULL;
	return pop(_readEnd);
}

const Segment *StreamBuffer::peekFront() const { return _readEnd; }

void StreamBuffer::discardFront()
{
	if (_readEnd)
		_buffManager.returnBuffers(popFront());
}
