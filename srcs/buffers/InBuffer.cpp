#include "InBuffer.hpp"
#include "ClientClosed.hpp"
#include <sys/socket.h>

ssize_t InBuffer::recvWrapper(int fd, void *buf, size_t len)
{
	return recv(fd, buf, len, 0);
}

InBuffer::InBuffer(int inFd, Segment::ReadFunc readFunc, BufferManager &bm) :
	StreamBuffer(bm),
	_inFd(inFd),
	_readFunc(readFunc)
{
}

bool InBuffer::ensureSpace()
{
	if (!_writeEnd || _writeEnd->writable() < (RECV_SIZE / 2)) {
		Segment *seg = _buffManager.getBuffer();
		if (!seg)
			return false;
		pushBack(seg);
	}
	return true;
}

StrView InBuffer::readIn()
{
	if (!ensureSpace())
		return StrView();
	size_t before = _writeEnd->readable();
	ssize_t n = _writeEnd->readFrom(_readFunc, _inFd);
	if (n == 0)
		return onEof();
	if (n < 0) // -1 handled as EAGAIN - Errors handeled through EPOLLERROR
		return StrView();
	_totalLen += n;
	return StrView(_writeEnd->data() + before, n);
}

StrView InBuffer::onEof() { throw ClientClosed(); }
