// StreamBuffer.hpp
#ifndef STREAMBUFFER_HPP
#define STREAMBUFFER_HPP

#include "BufferManager.hpp"
#include <cstddef>
#include <deque>

class StreamBuffer {
private:
	// Explicit disables
	StreamBuffer &operator=(const StreamBuffer &other);
	StreamBuffer(const StreamBuffer &other);

protected:
	struct Segment {
		char *_data;
		size_t _offset;
	};

	BufferManager &_pool;
	std::deque<Segment> _segments;
	size_t _totalOffset;
	size_t _totalLen;

public:
	StreamBuffer(BufferManager &buffManager);
	~StreamBuffer();

	// setters
	void push(char *buf, size_t used);

	// getters
	const char *cursorPtr() const;
	size_t lenLeft() const;
	void consume(size_t n);

	bool empty() const;
	size_t totalBytes() const;

	// Cleanup
	void returnBuffers();
};

#endif
