#pragma once

#include "Segment.hpp"
#include "StreamBuffer.hpp"

/* Buffered writer to a socket fd. Sends oldest segment,
 * pops when fully sent. Uses MSG_NOSIGNAL. */
class OutBuffer : public StreamBuffer
{
public:
	OutBuffer(int outFd, BufferManager &bm);

	ssize_t writeOut();

private:
	OutBuffer();
	OutBuffer(const OutBuffer &other);
	OutBuffer &operator=(const OutBuffer &other);

	const int _outFd;

	static ssize_t sendWrapper(int fd, const void *buf, size_t len);
};
