#include "InOutBuffer.hpp"
#include <sys/socket.h>

InOutBuffer::InOutBuffer(int inFd, Segment::ReadFunc readFunc, int outFd,
						 BufferManager &bm) :
	InBuffer(inFd, readFunc, bm),
	_outFd(outFd)
{
}

ssize_t InOutBuffer::sendWrapper(int fd, const void *buf, size_t len)
{
	return send(fd, buf, len, MSG_NOSIGNAL);
}

ssize_t InOutBuffer::writeOut()
{
	if (!_readEnd || _readEnd->allSent())
		return 0;
	ssize_t n = _readEnd->sendTo(sendWrapper, _outFd);
	if (n > 0 && _readEnd->allSent())
		_buffManager.returnBuffers(popFront());
	return n;
}
