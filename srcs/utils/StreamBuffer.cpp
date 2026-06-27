#include "StreamBuffer.hpp"

StreamBuffer::StreamBuffer(BufferManager &buffManager) :
	_pool(buffManager),
	_totalOffset(0),
	_totalLen(0) {}

StreamBuffer::~StreamBuffer() { returnBuffers(); }

void StreamBuffer::push(char *buf, size_t used) {
	Segment segment;
	segment._data = buf;
	segment._offset = used;
	_segments.push_back(segment);
	_totalLen += used;
}

const char *StreamBuffer::cursorPtr() const {
	if (_segments.empty())
		return NULL;
	return _segments.front()._data + _totalOffset;
}

size_t StreamBuffer::lenLeft() const {
	if (_segments.empty())
		return 0;
	return _segments.front()._offset - _totalOffset;
}

void StreamBuffer::consume(size_t n) {
	_totalOffset += n;

	while (!_segments.empty() && _totalOffset >= _segments.front()._offset) {
		_totalOffset -= _segments.front()._offset;
		_pool.returnBuffer(_segments.front()._data);
		_segments.pop_front();
	}
}

bool StreamBuffer::empty() const { return _segments.empty(); }

size_t StreamBuffer::totalBytes() const { return _totalLen; }

void StreamBuffer::returnBuffers() {
	for (size_t i = 0; i < _segments.size(); i++)
		_pool.returnBuffer(_segments[i]._data);
	_segments.clear();
	_totalOffset = 0;
	_totalLen = 0;
}
