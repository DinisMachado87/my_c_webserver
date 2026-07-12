#include "ChunkDecoder.hpp"
#include "ChunkDecodeError.hpp"
#include "Hex.hpp"
#include <unistd.h>

ChunkDecoder::ChunkDecoder(int inFd, int outFd, BufferManager &bm,
						   const StrView &leftover) :
	_input(inFd, InBuffer::recvWrapper, bm),
	_outFd(outFd),
	_state(PARSE_SIZE),
	_hexLen(0),
	_payloadRemaining(0),
	_cursor(0)
{
	if (!leftover.empty())
		_input.append(leftover);
}

ssize_t ChunkDecoder::decodeAndFlush()
{
	ssize_t total = 0;

	while (_state != FINISH) {
		popIfConsumed();
		const Segment *seg = _input.peekFront();
		if (!seg || _cursor >= seg->readable())
			return total;

		switch (_state) {
		case PARSE_SIZE:
			if (parseHexDigits())
				_state = SIZE_LF;
			continue;

		case SIZE_LF:
			if (expectChar('\n'))
				_state = (_payloadRemaining == 0) ? FINISH : PAYLOAD;
			continue;

		case PAYLOAD:
			if (!writePayload(total))
				return total;
			if (_payloadRemaining == 0)
				_state = TRAIL_CR;
			continue;

		case TRAIL_CR:
			if (expectChar('\r'))
				_state = TRAIL_LF;
			continue;

		case TRAIL_LF:
			if (expectChar('\n'))
				_state = PARSE_SIZE;
			continue;

		case FINISH:
			break;
		}
	}
	return total;
}

bool ChunkDecoder::done() const { return _state == FINISH; }
StrView ChunkDecoder::readIn() { return _input.readIn(); }

/* Helpers */
void ChunkDecoder::popIfConsumed()
{
	const Segment *seg = _input.peekFront();
	if (seg && _cursor >= seg->readable()) {
		_input.discardFront();
		_cursor = 0;
	}
}

bool ChunkDecoder::parseHexDigits()
{
	const Segment *seg = _input.peekFront();
	const char *data = seg->data();
	size_t readable = seg->readable();

	while (_cursor < readable) {
		char c = data[_cursor];
		if (c == '\r') {
			_cursor++;
			Hex hex;
			if (!hex.parseFromStr(_hexBuf, _hexLen))
				throw ChunkDecodeError("invalid chunk size");
			_payloadRemaining = hex.value();
			_hexLen = 0;
			return true;
		}
		if (_hexLen >= 16)
			throw ChunkDecodeError("chunk size exceeds 16 hex digits");
		_hexBuf[_hexLen++] = c;
		_cursor++;
	}
	return false;
}

bool ChunkDecoder::expectChar(char expected)
{
	const Segment *seg = _input.peekFront();
	if (_cursor >= seg->readable())
		return false;
	if (seg->data()[_cursor] != expected)
		throw ChunkDecodeError("unexpected byte in chunk framing");
	_cursor++;
	return true;
}

bool ChunkDecoder::writePayload(ssize_t &total)
{
	const Segment *seg = _input.peekFront();
	size_t available = seg->readable() - _cursor;
	if (available > _payloadRemaining)
		available = _payloadRemaining;

	ssize_t bytesWritten = ::write(_outFd, seg->data() + _cursor, available);
	if (bytesWritten <= 0) {
		if (total == 0)
			total = bytesWritten;
		return false;
	}

	size_t written = static_cast<size_t>(bytesWritten);
	_cursor += written;
	_payloadRemaining -= written;
	total += bytesWritten;
	return true;
}
