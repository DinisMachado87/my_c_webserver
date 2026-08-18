#pragma once
#include "webServ.hpp"
#include <cstddef>
#include <sys/types.h>

/* Owns the read-side syscall choice for a buffer. recv for sockets,
 * read for pipes/files. Kind fixed at construction. */
class Reader
{
public:
	/* Constructors */
	explicit Reader(e_FdType fdType, const int fd);

	/* Methods */
	ssize_t readIn(void *buf, size_t len) const;

private:
	Reader();

	/* State */
	const e_FdType _fdType;
	const int _fd;
};
