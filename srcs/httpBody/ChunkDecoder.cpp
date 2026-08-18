#include "ChunkDecoder.hpp"
#include "HttpStatus.hpp"
#include "Segment.hpp"
#include "StrView.hpp"
#include <cstddef>

ChunkDecoder::ChunkDecoder(const Reader &reader, const Writer &writer,
						   BufferManager &pool) :
	ScatterIOBuffer(reader, writer, pool),
	_state(SIZE),
	_remainingChunkLen(0),
	_cursor(0),
	_scratchLen(0),
	_expectLf(false)
{
}

ChunkDecoder::~ChunkDecoder() {}

/* Methods */
ssize_t ChunkDecoder::writeOut()
{
	if (_state == FINISH || !takeSegment())
		return 0;

	if (!_iovSections.loadedSections())
		loadSectionsToSend();
	ssize_t sent = sendSections();
	releaseSegment();
	return sent;
}

// Walks framing until the segment runs out, capacity fills, or terminator.
void ChunkDecoder::loadSectionsToSend()
{
	const StrView dataToSend = _curSegment->writtenView();
	const size_t dataSize = dataToSend.size();

	while (_cursor < dataSize && _iovSections.loadedSections() < MAX_SECTIONS) {
		switch (_state) {
		case SIZE:
			if (!parseHexStr(dataToSend))
				return; // line straddles, digits stashed
			break;
		case DATA:
			takePayload(dataToSend);
			break;
		case DATA_TRAIL:
			skipTrailer(dataToSend);
			break;
		case FINISH:
			return;
		}
	}
}

bool ChunkDecoder::done() const { return _state == FINISH; }

/* Protected Methods */
bool ChunkDecoder::segmentSpent() const
{
	return !_iovSections.loadedSections()
		   && _cursor >= _curSegment->writtenView().size();
}

bool ChunkDecoder::parseHexStr(const StrView &writtenView)
{
	while (_cursor < writtenView.size()) {
		char c = writtenView.data()[_cursor++];

		if (_expectLf && c != '\n')
			throw HttpStatus::BAD_REQUEST;

		switch (c) {
		case '\r':
			_expectLf = true;
			break;
		case '\n': // both '\n' are "\r\n" as Newline
			return commitSize();
		default:
			if (_scratchLen >= SCRATCH_SIZE)
				throw HttpStatus::BAD_REQUEST;
			_sizeScratch[_scratchLen++] = c;
		}
	}
	return false;
}

// Digits complete. Zero size ends the body.
bool ChunkDecoder::commitSize()
{
	if (!_hex.parseFromStr(_sizeScratch, _scratchLen))
		throw HttpStatus::BAD_REQUEST;
	_scratchLen = 0;
	_expectLf = false;
	_remainingChunkLen = _hex.value();
	_state = (_remainingChunkLen == 0) ? FINISH : DATA;
	return true;
}

void ChunkDecoder::takePayload(const StrView &view)
{
	size_t available = view.size() - _cursor;
	size_t len
		= (_remainingChunkLen < available) ? _remainingChunkLen : available;

	_iovSections.add(StrView(view.data() + _cursor, len));
	_cursor += len;
	_remainingChunkLen -= len;
	if (_remainingChunkLen == 0)
		_state = DATA_TRAIL;
}

// Skips the CRLF closing a chunk. '\n' ends it.
void ChunkDecoder::skipTrailer(const StrView &writtenView)
{
	char c = writtenView.data()[_cursor++];
	if (c == '\n')
		_state = SIZE;
	else if (c != '\r')
		throw HttpStatus::BAD_REQUEST;
}

bool ChunkDecoder::takeSegment()
{
	if (_curSegment)
		return true;
	if (!IOBuffer::takeSegment())
		return false;
	_cursor = 0;
	return true;
}
