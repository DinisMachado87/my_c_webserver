#include "StackBuffer.hpp"

StackBuffer::StackBuffer(BufferManager &buffManager) :
	SegmentList(buffManager)
{
}

Segment *StackBuffer::popBack()
{
	if (!_writeEnd)
		return NULL;
	return pop(_writeEnd);
}
