#pragma once

#include "SegmentList.hpp"

/* LIFO view over a SegmentList. Pops from the write end. */
class StackBuffer : public SegmentList
{
private:
	StackBuffer &operator=(const StackBuffer &other);
	StackBuffer(const StackBuffer &other);

public:
	StackBuffer(BufferManager &buffManager);

	// Returns newest segment, or NULL if empty.
	Segment *popBack();
};
