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
