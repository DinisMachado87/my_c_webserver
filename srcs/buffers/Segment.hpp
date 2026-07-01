#pragma once

#include "webServ.hpp"
#include <cstddef>

/* Fixed-size buffer with intrusive linked list pointers,
 * so segments can be chained without external allocations
 * for pointers containers or nodes.
 *
 * Manages its own read/write operations and cursors —
 * callers never touch raw data (returns char * as const). */
class Segment
{
public:
	typedef ssize_t (*ReadFunc)(int, void *, size_t);
	typedef ssize_t (*WriteFunc)(int, const void *, size_t);

	Segment *_prev;
	Segment *_next;

	Segment();

	void reset();
	void poison(); // Nulls buffer content.

	/* Data in*/
	// Reads fd into remaining space. Returns byte count or <= 0 on error/EOF.
	ssize_t readFrom(ReadFunc fn, int fd);
	// Copies up to writable() bytes from src. Returns bytes actually copied.
	size_t copyIn(const char *src, size_t len);

	/* Data out */
	// Writes unsent data to fd. Returns byte count or <= 0 on error.
	ssize_t sendTo(WriteFunc fn, int fd);
	// True when all readable data has been sent.
	bool allSent() const;

	// Read-only access
	const char *data() const;
	size_t readable() const;
	size_t writable() const;

private:
	char _data[RECV_SIZE];
	size_t _usedLen;
	size_t _sent;
};
