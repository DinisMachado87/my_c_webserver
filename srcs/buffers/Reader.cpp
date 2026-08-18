#include "Reader.hpp"
#include <sys/socket.h>
#include <unistd.h>

Reader::Reader(e_FdType fdType, int fd) :
	_fdType(fdType),
	_fd(fd)
{
}

/* Methods */
ssize_t Reader::readIn(void *buf, size_t len) const
{
	switch (_fdType) {
	case FD_SOCKET:
		return recv(_fd, buf, len, 0);
	case FD_FILE:
		return ::read(_fd, buf, len);
	}
	return -1;
}
