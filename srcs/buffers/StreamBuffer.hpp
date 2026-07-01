#pragma once

#include "SegmentList.hpp"

// FIFO view over a SegmentList. Pops from the read end.
class StreamBuffer : public SegmentList
{
private:
	StreamBuffer &operator=(const StreamBuffer &other);
	StreamBuffer(const StreamBuffer &other);

public:
	StreamBuffer(BufferManager &buffManager);

	// Removes and returns the oldest segment. Returns NULL if empty.
	Segment *popFront();
};
