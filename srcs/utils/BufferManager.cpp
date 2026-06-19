#include "BufferManager.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <vector>

// Public constructors and destructors
BufferManager::BufferManager() :
	_nextAllocMultiplier(8) {}

BufferManager::~BufferManager() {
	for (size_t i = 0; i < _slabAllocations.size(); i++)
		delete _slabAllocations[i];
}

// Public Methods
void BufferManager::allocate() {
	char *ptr = new char[_nextAllocMultiplier * RECV_SIZE];
	_slabAllocations.push_back(ptr);

	for (size_t i = 0; i < _nextAllocMultiplier; i++) {
		_buffers.push(ptr);
		ptr += RECV_SIZE;
	}
	_nextAllocMultiplier *= 2;
}

char *BufferManager::getBuffer() {
	if (_buffers.empty())
		allocate();
	if (_buffers.empty())
		return NULL;

	char *buffer = _buffers.top();
	_buffers.pop();
	return buffer;
}

char *BufferManager::returnBuffer(char *buffer) {
	_buffers.push(buffer);
	return NULL;
}

void BufferManager::returnBuffers(std::vector<StrView> &buffers) {
	for (size_t i = 0; i < buffers.size(); i++)
		returnBuffer((char *)buffers[i].data());
}
