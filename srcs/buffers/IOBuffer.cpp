#include "IOBuffer.hpp"
#include "BufferManager.hpp"
#include "Segment.hpp"
#include "Writer.hpp"

IOBuffer::IOBuffer(int inFd, Reader::FdType inFdType, int outFd,
				   IOBuffer::e_outFdType outFdType, BufferManager &pool) :
	IBuffer(inFd, inFdType, pool),
	_writer(static_cast<Writer::fdType>(outFdType), outFd)
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
