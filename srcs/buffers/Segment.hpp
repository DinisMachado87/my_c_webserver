#pragma once

#include "webServ.hpp"
#include "StrView.hpp"
#include <cstddef>
#include <sys/types.h>

// Forward declaration: Segment owns its cursors, so it drives the syscall
// itself. Reader/Writer never include Segment — reference is one-directional.
class Reader;
class Writer;

class BufferManager;
class SegmentList;

/* Fixed-size buffer with intrusive links — chains without
 * allocating. Owns its cursors, exposes views not raw data.
 * Links + ctor private: only pool and list can build/splice. */
class Segment
{
private:
	/* State */
	char _data[RECV_SIZE];
	size_t _written;
	size_t _sent;
	Segment *_prev;
	Segment *_next;

	// Only friends can build and link
	friend class SegmentList;
	friend class BufferManager;
	/* Constructors */
	Segment();
	/* Link primitives — patch neighbors, never touch list endpoints */
	void linkNext(Segment *node);
	void linkPrev(Segment *node);
	void unlink();
	void clearCursors();

public:
	/* Methods */
	void reset();
	void poison();

	ssize_t readFrom(const Reader &reader);
	// Returns bytes copied, truncated to writable().
	size_t copyIn(const char *src, size_t len);

	ssize_t sendTo(const Writer &writer);
	bool allSent() const;

	StrView writtenView() const;
	StrView unsentView() const;
	StrView lastWritten(size_t n) const;

	size_t readable() const;
	size_t writable() const;
	size_t used() const;
};
