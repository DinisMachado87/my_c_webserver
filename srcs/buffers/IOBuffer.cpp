#include "IOBuffer.hpp"
#include "BufferManager.hpp"
#include "Segment.hpp"
#include "Writer.hpp"

IOBuffer::IOBuffer(const Reader &reader, const Writer &writer,
				   BufferManager &pool) :
	IBuffer(reader, pool),
	_writer(writer)
{
}

IOBuffer::~IOBuffer() {}

/* Methods */
ssize_t IOBuffer::writeOut()
{
	Segment *seg = _segList.popHead();
	if (!seg)
		return 0;

	ssize_t bytesRead = seg->sendTo(_writer);
	if (seg->readable() == 0)
		_segPool.returnSegment(seg);
	else
		_segList.pushHead(seg);
	return bytesRead;
}
