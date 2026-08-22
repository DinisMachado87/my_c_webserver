#pragma once

#include "StrView.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <sys/types.h>

// Forward declaration: Segment owns its cursors, so it drives the syscall
// itself. Reader/Writer never include Segment — reference is one-directional.
class Reader;
class Writer;

class BufferManager;
class SegmentList;

/* Fixed-size buffer with intrusive links — chains without
 * allocating.
 *
 * Owns, tracks and returns its cursors, exposes views not raw data:
 * - writenView;
 * - unsentView;
 * - lastWritten;
 *
 * Links + constructor private: only pool and list can link. */
class Segment
{
private:
	/* State */
	char _data[RECV_SIZE];
	size_t _written;
	size_t _used;
	Segment *_prev;
	Segment *_next;

	// Only friends can build and link
	friend class SegmentList;
	friend class BufferManager;
	/* Constructors */
	Segment();
	/* Link */
	void linkNext(Segment *node);
	void linkPrev(Segment *node);
	void unlink();
	void clearCursors();

public:
	enum e_comparison { MATCH, MISMATCH, INCOMPLETE };

	/* Methods */
	void reset();
	void poison();

	ssize_t readFrom(const Reader &reader, size_t cap = 0);
	// Returns bytes copied. Truncates to writable().
	size_t copyIn(const char *src, size_t len);

	ssize_t sendTo(const Writer &writer);
	bool allSent() const;

	StrView writtenView() const;
	StrView unusedView() const;
	StrView lastWritten(size_t n) const;

	size_t readable() const;
	size_t writable() const;
	size_t used() const;

	void advanceUsed(size_t n);
	e_comparison compare(const StrView &expected) const;
};

inline std::ostream &operator<<(std::ostream &os, const Segment &seg)
{
	return os << seg.writtenView();
}
