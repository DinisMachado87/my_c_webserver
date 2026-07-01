#include "BufferManager.hpp"
#include "Segment.hpp"
#include <cstddef>
#include <cstring>
#include <vector>

// Public constructors and destructors
BufferManager::BufferManager() :
	_buffers(NULL),
	_multiplier(8)
{
}

BufferManager::~BufferManager()
{
	for (size_t i = 0; i < _slabAllocations.size(); i++)
		delete[] _slabAllocations[i];
}

// Public Methods
void BufferManager::allocate()
{
	Segment *segmentsPtr = new Segment[_multiplier];
	_slabAllocations.push_back(segmentsPtr);

	for (size_t i = 0; i < _multiplier; i++) {
		segmentsPtr->_next = _buffers;
		_buffers = segmentsPtr;
		segmentsPtr++;
	}
	_multiplier *= 2;
}

Segment *BufferManager::getBuffer()
{
	Segment *buffer = _buffers;
	if (!buffer) {
		allocate();
		buffer = _buffers;
		if (!buffer)
			return NULL;
	}
	_buffers = _buffers->_next;
	buffer->reset();
	return buffer;
}

Segment *BufferManager::returnBuffers(Segment *head)
{
	while (head) {
		Segment *next = head->_next;
		head->reset();
		head->poison();
		head->_next = _buffers;
		_buffers = head;
		head = next;
	}
	return NULL;
}
