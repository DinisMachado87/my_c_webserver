#pragma once

#include "Segment.hpp"
#include "StrView.hpp"
#include "StreamBuffer.hpp"
#include <cstddef>

/* Bidirectional I/O over a StreamBuffer. Type enum wires the correct
 * syscall wrappers at construction — no subclasses needed.
 * Allocates a new segment when the current one is more than half full
 * so the next read has meaningful space without a second syscall. */
class IOBuffer : public StreamBuffer
{
public:
	enum Type { REQUEST, SOCK_TO_FILE, FILE_TO_SOCK };

	IOBuffer(Type type, int inFd, int outFd, BufferManager &buffManager);

	// Reads from inFd. Returns a StrView over the new data, or empty on EAGAIN.
	// Throws ClientClosed on EOF.
	StrView readIn();

	// Writes oldest segment to outFd. Pops it when fully sent. Returns bytes
	// written, 0 if nothing to send, or < 0 on error.
	ssize_t writeOut();

private:
	// Explicit disables
	IOBuffer &operator=(const IOBuffer &other);
	IOBuffer(const IOBuffer &other);

	// State
	int _inFd;
	int _outFd;
	Segment::ReadFunc _readFunc;
	Segment::WriteFunc _writeFunc;

	// Methods
	bool ensureSpace();

	static ssize_t recvWrapper(int fd, void *buf, size_t len);
	static ssize_t sendWrapper(int fd, const void *buf, size_t len);
};
