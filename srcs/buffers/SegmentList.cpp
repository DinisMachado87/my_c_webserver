#include "SegmentList.hpp"
#include <cassert>

SegmentList::SegmentList(BufferManager &buffManager) :
	_buffManager(buffManager),
	_readEnd(NULL),
	_writeEnd(NULL),
	_totalLen(0)
{
}

SegmentList::~SegmentList()
{
	while (_readEnd)
		_buffManager.returnBuffers(pop(_readEnd));
}

void SegmentList::pushBack(Segment *seg)
{
	assert(seg && "pushBack: NULL segment");
	assert(!seg->_next && "pushBack: seg still linked (_next)");
	assert(!seg->_prev && "pushBack: seg still linked (_prev)");

	_totalLen += seg->readable();
	seg->_next = NULL;
	if (!_writeEnd) {
		seg->_prev = NULL;
		_readEnd = seg;
		_writeEnd = seg;
	} else {
		_writeEnd->_next = seg;
		seg->_prev = _writeEnd;
		_writeEnd = seg;
	}
}

Segment *SegmentList::pop(Segment *seg)
{
	assert(seg && "pop: NULL segment");

	if (seg->_prev)
		seg->_prev->_next = seg->_next;
	else
		_readEnd = seg->_next;

	if (seg->_next)
		seg->_next->_prev = seg->_prev;
	else
		_writeEnd = seg->_prev;

	_totalLen -= seg->used();
	seg->reset();
	return seg;
}
size_t SegmentList::totalLen() const { return _totalLen; }
bool SegmentList::empty() const { return _readEnd == NULL; }

void SegmentList::append(const char *data, size_t len)
{
	while (len > 0) {
		if (!_writeEnd || _writeEnd->writable() == 0) {
			Segment *seg = _buffManager.getBuffer();
			pushBack(seg);
		}
		size_t n = _writeEnd->copyIn(data, len);
		_totalLen += n;
		data += n;
		len -= n;
	}
}

void SegmentList::append(const StrView &sv) { append(sv.data(), sv.size()); }
