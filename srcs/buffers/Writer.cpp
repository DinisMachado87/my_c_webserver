#include "Writer.hpp"
#include <cstddef>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

Writer::Writer(fdType kind, int fd) :
	_fdType(kind),
	_fd(fd)
{
}

/* Methods */
ssize_t Writer::writeGather(struct iovec *iov, size_t iovLen) const
{
	switch (_fdType) {
	case SOCKET:
		return sendmsgWrite(iov, iovLen);
	case FILE:
		return writev(_fd, iov, iovLen);
	}
	return -1; // unreachable, silences warning
}

ssize_t Writer::writeOne(const void *buf, size_t len) const
{
	switch (_fdType) {
	case SOCKET:
		return send(_fd, buf, len, MSG_NOSIGNAL);
	case FILE:
		return ::write(_fd, buf, len);
	}
	return -1; // unreachable, silences warning
}

/* MSG_NOSIGNAL — client closed, EPIPE not SIGPIPE. */
ssize_t Writer::sendmsgWrite(struct iovec *iov, int cnt) const
{
	struct msghdr msg;
	std::memset(&msg, 0, sizeof(msg));
	msg.msg_iov = const_cast<struct iovec *>(iov);
	msg.msg_iovlen = cnt;
	return sendmsg(_fd, &msg, MSG_NOSIGNAL);
}
