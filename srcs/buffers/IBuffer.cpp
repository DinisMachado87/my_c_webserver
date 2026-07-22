#include "IBuffer.hpp"
#include "BufferManager.hpp"
#include "Segment.hpp"

IBuffer::IBuffer(int inFd, Reader::FdType inKind, BufferManager &pool) :
	_reader(inKind, inFd),
	_segList(pool),
	_pool(pool),
	_inClosed(false)
{
}

IBuffer::~IBuffer() {}

/* Methods */
ssize_t IBuffer::readIn()
{
	Segment *seg = _segList.popTail();
	if (!seg)
		seg = _pool.getSegment();

	else if (seg->writable() < RECV_SIZE / 3) {
		_segList.pushTail(seg);
		seg = _pool.getSegment();
	}
	ssize_t bytesRead = seg->readFrom(_reader);

	if (seg->readable() > 0)
		_segList.pushTail(seg);
	else
		_segList.pushToRecycleStack(seg);

	if (bytesRead == 0) {
		_inClosed = true;
		onEof();
	}
	return bytesRead;
}

bool IBuffer::inClosed() const { return _inClosed; }

bool IBuffer::done() const { return _inClosed && _segList.empty(); }

void IBuffer::onEof() {}
