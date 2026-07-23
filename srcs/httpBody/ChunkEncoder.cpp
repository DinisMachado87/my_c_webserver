#include "ChunkEncoder.hpp"
#include "BufferManager.hpp"
#include "Segment.hpp"
#include "StrView.hpp"
#include "Writer.hpp"

ChunkEncoder::ChunkEncoder(int inFd, Reader::FdType inFdType, int outFd,
						   IOBuffer::e_outFdType outFdType,
						   BufferManager &pool) :
	IOBuffer(inFd, inFdType, outFd, outFdType, pool),
	_state(FRAME),
	_curSegment(NULL)
{
}

ChunkEncoder::~ChunkEncoder()
{
	if (_curSegment)
		_curSegment = _segPool.returnSegment(_curSegment);
}

/* Methods */
ssize_t ChunkEncoder::writeOut()
{
	switch (_state) {
	case FRAME:
		return frameAndSend();
	case SEND:
		return resumeSend(FRAME);
	case SEND_TERMINATOR:
		return resumeSend(FINISH);
	case FINISH:
		return 0;
	}
	return 0;
}

bool ChunkEncoder::done() const { return _state == FINISH; }

/* Private Helpers */

ssize_t ChunkEncoder::frameAndSend()
{
	if (frameSegment()) {
		_state = SEND;
		return resumeSend(FRAME);
	}

	if (!_inClosed)
		return 0;

	frameTerminator();
	_state = SEND_TERMINATOR;
	return resumeSend(FINISH);
}

ssize_t ChunkEncoder::resumeSend(e_state nextState)
{
	ssize_t sent
		= _writer.writeGather(_iovSegments.iov(), _iovSegments.sectionsLeft());
	if (sent > 0)
		_iovSegments.advance(sent);
	if (_iovSegments.sectionsLeft())
		return sent; // partial, resume on next EPOLLOUT

	if (nextState == FRAME)
		_curSegment = _segPool.returnSegment(_curSegment);
	_state = nextState;
	return sent;
}

// builds [hex][\r\n][data][\r\n]
bool ChunkEncoder::frameSegment()
{
	_curSegment = _segList.popHead();
	if (!_curSegment)
		return false;

	StrView payload = _curSegment->unsentView();
	_hex.parseFromNum(payload.size());
	_iovSegments.reset();
	_iovSegments.add(StrView(_hex.str(), _hex.strLen()));
	_iovSegments.add(StrView("\r\n", 2));
	_iovSegments.add(payload);
	_iovSegments.add(StrView("\r\n", 2));
	return true;
}

void ChunkEncoder::frameTerminator()
{
	_iovSegments.reset();
	_iovSegments.add(StrView("0\r\n\r\n", 5));
}
