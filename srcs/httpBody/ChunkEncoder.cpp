#include "ChunkEncoder.hpp"
#include "Segment.hpp"
#include "StrView.hpp"

ChunkEncoder::ChunkEncoder(const Reader &reader, const Writer &writer,
						   BufferManager &pool) :
	ScatterIOBuffer(reader, writer, pool),
	_state(FRAME)
{
}

ChunkEncoder::~ChunkEncoder() {}

/* Methods */
ssize_t ChunkEncoder::writeOut()
{
	for (;;) {
		switch (_state) {
		case FRAME:
			if (frameSegment())
				_state = SEND;
			else if (_inClosed) {
				frameTerminator();
				_state = SEND_TERMINATOR;
			} else
				return 0; // nothing framed, nothing to close with
			break;
		case SEND:
			return resumeSend(FRAME);
		case SEND_TERMINATOR:
			return resumeSend(FINISH);
		case FINISH:
			return 0;
		}
	}
}

/* Protected Methods */
ssize_t ChunkEncoder::resumeSend(e_state nextState)
{
	ssize_t sent = sendSections();
	if (_iovSections.loadedSections())
		return sent; // partial, resume on next EPOLLOUT

	releaseSegment();
	_state = nextState;
	return sent;
}

// builds [hex][\r\n][data][\r\n]
bool ChunkEncoder::frameSegment()
{
	if (!takeSegment())
		return false;

	StrView payload = _curSegment->unusedView();
	_hex.parseFromNum(payload.size());
	_iovSections.reset();
	_iovSections.add(StrView(_hex.str(), _hex.strLen()));
	_iovSections.add(StrView("\r\n", 2));
	_iovSections.add(payload);
	_iovSections.add(StrView("\r\n", 2));
	return true;
}

void ChunkEncoder::frameTerminator()
{
	_iovSections.reset();
	_iovSections.add(StrView("0\r\n\r\n", 5));
}

bool ChunkEncoder::segmentSpent() const
{
	return !_iovSections.loadedSections();
}

bool ChunkEncoder::done() const { return _state == FINISH; }
