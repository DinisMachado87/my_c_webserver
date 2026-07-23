#include "IBuffer.hpp"
#include "BufferManager.hpp"
#include "Segment.hpp"

IBuffer::IBuffer(const Reader &reader, BufferManager &pool) :
	_reader(reader),
	_segList(pool),
	_segPool(pool),
	_inClosed(false)
{
}

IBuffer::~IBuffer() {}

/* Methods */
ssize_t IBuffer::readIn()
{
	Segment *seg = _segList.popTail();
	if (!seg)
		seg = _segPool.getSegment();

	else if (seg->writable() < RECV_SIZE / 3) {
		_segList.pushTail(seg);
		seg = _segPool.getSegment();
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
