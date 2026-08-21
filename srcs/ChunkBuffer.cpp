#include "ChunkBuffer.hpp"
#include "BufferManager.hpp"
#include "Segment.hpp"
#include "Writer.hpp"

ScatterIOBuffer::ScatterIOBuffer(int inFd, Reader::FdType inFdType, int outFd,
								 e_outFdType outFdType, BufferManager &pool) :
	IOBuffer(inFd, inFdType, outFd, outFdType, pool),
	_curSegment(NULL)
{
}

ScatterIOBuffer::~ScatterIOBuffer()
{
	if (_curSegment)
		_curSegment = _segPool.returnSegment(_curSegment);
}

/* Protected Helpers */

ssize_t ScatterIOBuffer::resumeSend()
{
	ssize_t sent = _writer.writeGather(_iovSections.iov(),
									   _iovSections.loadedSections());
	if (sent > 0)
		_iovSections.advance(sent);
	if (_iovSections.loadedSections())
		return sent; // partial, resume on next EPOLLOUT

	releaseSegment();
	return sent;
}

void ScatterIOBuffer::releaseSegment()
{
	if (_curSegment && segmentSpent())
		_curSegment = _segPool.returnSegment(_curSegment);
}
