#pragma once
#include <cstddef>
#include <sys/types.h>

/* Owns the read-side syscall choice for a buffer. recv for sockets,
 * read for pipes/files. Kind fixed at construction. */
class Reader
{
public:
	/* State */
	enum FdType { SOCKET, FILE, NONE };

	/* Constructors */
	explicit Reader(FdType fdType, const int fd);

	/* Methods */
	ssize_t readIn(void *buf, size_t len) const;

private:
	Reader();

	/* State */
	const FdType _fdType;
	const int _fd;
};
