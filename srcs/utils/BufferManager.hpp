#ifndef BUFFERMANAGER_HPP
#define BUFFERMANAGER_HPP

#include "StrView.hpp"
#include <cstddef>
#include <stack>
#include <vector>

class BufferManager {
private:
	std::vector<char *> _slabAllocations;
	std::stack<char *, std::vector<char *> > _buffers;
	size_t _nextAllocMultiplier;

	// Explicit disables
	BufferManager &operator=(const BufferManager &other);
	BufferManager(const BufferManager &other);
	// Private Methods
	void allocate();

public:
	// Constructors and destructors
	BufferManager();
	~BufferManager();

	// Methods
	void returnBuffers(std::vector<StrView> &buffers);
	char *returnBuffer(char *buffer);
	char *getBuffer();
};

#endif
