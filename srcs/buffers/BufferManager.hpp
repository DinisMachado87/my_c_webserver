#pragma once

#include "Segment.hpp"
#include <cstddef>
#include <vector>

/* Pool allocator for Segments. Allocates in slabs (8, 16, 32, ...)
 * and recycles via an intrusive free list to minimise malloc calls.*/
class BufferManager
{
public:
	BufferManager();
	~BufferManager();

	// Returns a reset segment for use. Allocates new slab if pool empty.
	Segment *getBuffer();

	// Returns Segments to pool - follows _next chain, resets and poisons.
	// Callers don't need to unlink segments first.
	// Always returns NULL for 1 line reassignement of caller.
	Segment *returnBuffers(Segment *buffersHead);

private:
	std::vector<Segment *> _slabAllocations;
	Segment *_buffers;
	size_t _multiplier;

	/* Explicit disables */
	BufferManager &operator=(const BufferManager &other);
	BufferManager(const BufferManager &other);

	void allocate();
};
