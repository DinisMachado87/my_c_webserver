#pragma once
#include "IOBuffer.hpp"
#include "ScatterList.hpp"
#include <sys/types.h>

/* Shared plumbing for the chunked framing buffers. Holds one Segment
 * across epoll wakes — iovecs in _sections point into its _data, so the
 * segment is returned to the pool only once the batch drains and the
 * derived class reports it spent. One writeGather per wake, partial sends
 * resume from the surviving sections. */
class ScatterIOBuffer : public IOBuffer
{
private:
	/* Constructors */
	ScatterIOBuffer();
	ScatterIOBuffer(const ScatterIOBuffer &other);
	ScatterIOBuffer &operator=(const ScatterIOBuffer &other);

protected:
	static const int MAX_SECTIONS = 8;

	/* Constructors */
	ScatterIOBuffer(int inFd, Reader::FdType inFdType, int outFd,
					e_outFdType outFdType, BufferManager &pool);

	/* State */
	Segment *_curSegment;
	ScatterList<MAX_SECTIONS> _iovSections;

	/* Methods */
	// Sends _sections. Releases the segment when drained and spent.
	ssize_t resumeSend();
	// True when the derived parser has no further use for _curSegment.
	virtual bool segmentSpent() const = 0;
	void releaseSegment();

public:
	/* Destructors */
	virtual ~ScatterIOBuffer();
};
