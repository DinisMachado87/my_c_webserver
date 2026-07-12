#pragma once

#include "Segment.hpp"
#include "StrView.hpp"
#include "StreamBuffer.hpp"

/* Buffered reader from an fd. Reads into memory pool segments.
 * Override onEof() to customize EOF handling —
 * default throws ClientClosed (socket disconnect). */
class InBuffer : public StreamBuffer
{
public:
	InBuffer(int inFd, Segment::ReadFunc readFunc, BufferManager &bm);

	static ssize_t recvWrapper(int fd, void *buf, size_t len);

	StrView readIn();

protected:
	virtual StrView onEof();
	bool ensureSpace();

	const int _inFd;
	Segment::ReadFunc _readFunc;

private:
	InBuffer();
	InBuffer(const InBuffer &other);
	InBuffer &operator=(const InBuffer &other);
};
