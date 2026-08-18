#pragma once
#include "IOBuffer.hpp"
#include "ScatterList.hpp"
#include <sys/types.h>

/* Shared machinery for the two reframing buffers.
 * Adds a ScatterList over IOBuffer's held segment
 * — iovecs in _iovSections point into its _data,
 * so sendSections() must not release the segment while sections remain.
 * Capacity 8 covers the decoder's many-chunks-per-segment case;
 * the encoder uses 5 ([hex][\r\n][data][\r\n] plus terminator slack). */
class ScatterIOBuffer : public IOBuffer
{
private:
	/* Constructors */
	ScatterIOBuffer();
	ScatterIOBuffer(const ScatterIOBuffer &other);
	ScatterIOBuffer &operator=(const ScatterIOBuffer &other);

protected:
	static const int MAX_SECTIONS = 8;

	/* State */
	ScatterList<MAX_SECTIONS> _iovSections;

	/* Constructors */
	ScatterIOBuffer(const Reader &reader, const Writer &writer,
					BufferManager &pool);

	/* Methods */
	// One gather write. Releases the segment when the sections drain.
	// Returns bytes sent, or the syscall's error.
	ssize_t sendSections();
	virtual bool segmentSpent() const = 0;

public:
	virtual ~ScatterIOBuffer();
};
