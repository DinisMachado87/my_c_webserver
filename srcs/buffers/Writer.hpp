#pragma once
#include "webServ.hpp"
#include <cstddef>
#include <sys/uio.h>

/* Owns the write-side syscall choice for a buffer. One instance per
 * out-capable buffer. Kind fixed at construction. single() writes one
 * contiguous span; gather() writes caller-built iovec sections.
 * NONE: no out fd (read-only/drain buffer) */
class Writer
{
public:
	/* Constructors */
	explicit Writer(e_FdType fdType, int fd);
	Writer(const Writer &other);

	/* Methods */
	ssize_t writeGather(const struct iovec *iov, size_t iovLen) const;
	ssize_t writeOne(const void *buf, size_t len) const;

private:
	/* Explicit disables */
	Writer();

	/* State */
	const e_FdType _fdType;
	const int _fd;

	/* Methods */
	ssize_t sendmsgWrite(const struct iovec *iov, int cnt) const;
};
