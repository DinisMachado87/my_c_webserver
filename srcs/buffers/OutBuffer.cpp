#include "OutBuffer.hpp"
#include <sys/socket.h>

OutBuffer::OutBuffer(int outFd, BufferManager &bm) :
	StreamBuffer(bm),
	_outFd(outFd)
{
}

ssize_t OutBuffer::sendWrapper(int fd, const void *buf, size_t len)
{
	return send(
		fd, buf, len,
		MSG_NOSIGNAL); // MSG_NOSIGNAL - if closed the connection, returns -1
					   // with errno=EPIPE instead of SIGPIPE.
}

ssize_t OutBuffer::writeOut()
{
	if (!_readEnd || _readEnd->allSent())
		return 0;
	ssize_t n = _readEnd->sendTo(sendWrapper, _outFd);
	if (n > 0 && _readEnd->allSent())
		_buffManager.returnBuffers(popFront());
	return n;
}
