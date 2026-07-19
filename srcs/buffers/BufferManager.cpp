#include "BufferManager.hpp"
#include "Segment.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <cstring>
#include <vector>

// Public constructors and destructors
BufferManager::BufferManager() :
	_buffers(NULL),
	_multiplier(SLAB_START_SIZE)
{
	// 40 doublings exceeds addressable memory; vector never grows past reserve.
	_slabs.reserve(40);
}

BufferManager::~BufferManager()
{
	for (size_t i = 0; i < _slabs.size(); i++)
		delete[] _slabs[i];
}

// Public Methods
Segment *BufferManager::getSegment()
{
	if (!_buffers)
		allocate();
	Segment *seg = _buffers;
	_buffers = _buffers->_next;
	seg->_next = NULL;
	return seg;
}

Segment *BufferManager::returnSegment(Segment *seg)
{
	seg->reset();
	seg->poison();
	seg->_next = _buffers;
	_buffers = seg;
	return NULL;
}

// Private Methods
void BufferManager::allocate()
{
	Segment *segmentsPtr = new Segment[_multiplier];
	_slabs.push_back(segmentsPtr);

	for (size_t i = 0; i < _multiplier; i++) {
		segmentsPtr->_next = _buffers;
		_buffers = segmentsPtr;
		segmentsPtr++;
	}
	_multiplier *= 2;
}
