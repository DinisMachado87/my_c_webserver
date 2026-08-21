#pragma once

#include "Reader.hpp"
#include "SegmentList.hpp"
#include <sys/types.h>

class BufferManager;

/* Input only buffer. Fills a Segment chain from _reader one segment per
 * readIn() (level-triggered). Owns no Writer —
 * IOBuffer derives adding output functionality. Input EOF sets _inClosed
 * but does not end the object: chain drains until done(). */
class IBuffer
{
public:
	/* Constructors */
	IBuffer(const Reader &reader, BufferManager &pool);
	virtual ~IBuffer();

	/* Methods */
	// Returns bytes read, 0 on EOF, -1 on error.
	virtual ssize_t readIn();

	// Only meaningful before the first readIn() — afterwards the bytes land
	// behind whatever the reader already fetched. Returns bytes written.
	size_t append(const StrView &data);

	bool inClosed() const;
	virtual bool done() const;

protected:
	/* Called once on input EOF. Default does nothing. */
	virtual void onEof();

	/* State */
	Reader _reader;
	SegmentList _segList;
	BufferManager &_segPool;
	bool _inClosed;

private:
	IBuffer();
	IBuffer(const IBuffer &other);
	IBuffer &operator=(const IBuffer &other);
};

inline IBuffer &operator<<(IBuffer &buf, const StrView &data)
{
	buf.append(data);
	return buf;
}
