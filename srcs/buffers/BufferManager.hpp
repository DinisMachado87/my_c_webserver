#pragma once

#include "Segment.hpp"
#include <cstddef>
#include <vector>

/* Pool allocator for Segments. Allocates in slabs (8, 16, 32, ...)
 * and recycles via an intrusive free list to minimise malloc calls. */
class BufferManager
{
public:
	BufferManager();
	~BufferManager();

	// Returns a reset segment for use. Allocates new slab if pool empty.
	Segment *getSegment();

	// Resets + poisons one seg, pushes to free list.
	// Always returns NULL for 1 line reassignement of caller.
	Segment *returnSegment(Segment *seg);

private:
	std::vector<Segment *> _slabs;
	Segment *_buffers;
	size_t _multiplier;

	/* Explicit disables */
	BufferManager &operator=(const BufferManager &other);
	BufferManager(const BufferManager &other);

	void allocate();
};
