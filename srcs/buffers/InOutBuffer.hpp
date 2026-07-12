#pragma once

#include "InBuffer.hpp"
#include "Segment.hpp"

/* Reads from inFd (pipe/file), writes to outFd (socket).
 * writeOut() is virtual — ChunkEncoder overrides it
 * with scatter-gather sendmsg. */
class InOutBuffer : public InBuffer
{
public:
	InOutBuffer(int inFd, Segment::ReadFunc readFunc, int outFd,
				BufferManager &bm);

	virtual ssize_t writeOut();

protected:
	const int _outFd;

private:
	InOutBuffer();
	InOutBuffer(const InOutBuffer &other);
	InOutBuffer &operator=(const InOutBuffer &other);

	static ssize_t sendWrapper(int fd, const void *buf, size_t len);
};
