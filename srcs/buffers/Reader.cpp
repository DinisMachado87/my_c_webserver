#include "Reader.hpp"
#include <sys/socket.h>
#include <unistd.h>

Reader::Reader(FdType fdType, int fd) :
	_fdType(fdType),
	_fd(fd)
{
}

/* Methods */
ssize_t Reader::readIn(void *buf, size_t len) const
{
	switch (_fdType) {
	case SOCKET:
		return recv(_fd, buf, len, 0);
	case FILE:
		return ::read(_fd, buf, len);
	case NONE:
		return 0;
	}
	return -1;
}
