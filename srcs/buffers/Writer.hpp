#pragma once
#include <cstddef>
#include <sys/uio.h>

/* Owns the write-side syscall choice for a buffer. One instance per
 * out-capable buffer. Kind fixed at construction. single() writes one
 * contiguous span; gather() writes caller-built iovec sections. */
class Writer
{
public:
	Writer();

	/* State */
	enum fdType { SOCKET, FILE };

	/* Constructors */
	explicit Writer(fdType fdType, int fd);

	/* Methods */
	ssize_t writeGather(struct iovec *iov, size_t iovLen) const;
	ssize_t writeOne(const void *buf, size_t len) const;

private:
	/* State */
	const fdType _fdType;
	const int _fd;

	/* Methods */
	ssize_t sendmsgWrite(struct iovec *iov, int cnt) const;
};
