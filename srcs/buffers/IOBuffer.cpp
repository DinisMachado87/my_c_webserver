#include "IOBuffer.hpp"
#include "Exception.hpp"
#include <cassert>
#include <sys/socket.h>
#include <unistd.h>

ssize_t IOBuffer::recvWrapper(int fd, void *buf, size_t len)
{
	return recv(fd, buf, len, 0);
}

ssize_t IOBuffer::sendWrapper(int fd, const void *buf, size_t len)
{
	return send(fd, buf, len, 0);
}

IOBuffer::IOBuffer(Type type, int inFd, int outFd, BufferManager &bm) :
	StreamBuffer(bm),
	_inFd(inFd),
	_outFd(outFd)
{
	switch (type) {
	case REQUEST:
		_readFunc = recvWrapper;
		_writeFunc = NULL;
		break;
	case SOCK_TO_FILE:
		_readFunc = recvWrapper;
		_writeFunc = ::write;
		break;
	case FILE_TO_SOCK:
		_readFunc = ::read;
		_writeFunc = sendWrapper;
		break;
	}
}

bool IOBuffer::ensureSpace()
{
	if (!_writeEnd || _writeEnd->writable() < (RECV_SIZE / 2)) {
		Segment *seg = _buffManager.getBuffer();
		if (!seg)
			return false;
		pushBack(seg);
	}
	return true;
}

StrView IOBuffer::readIn()
{
	if (!ensureSpace())
		return StrView();
	size_t before = _writeEnd->readable();
	ssize_t n = _writeEnd->readFrom(_readFunc, _inFd);
	if (n <= 0) {
		if (n == 0)
			throw ClientClosed();
		return StrView();
	}
	_totalLen += n;
	return StrView(_writeEnd->data() + before, n);
}

ssize_t IOBuffer::writeOut()
{
	assert(_writeFunc && "writeOut called on read-only buffer");

	if (!_readEnd || _readEnd->allSent())
		return 0;
	ssize_t n = _readEnd->sendTo(_writeFunc, _outFd);
	if (n > 0 && _readEnd->allSent())
		_buffManager.returnBuffers(popFront());
	return n;
}
