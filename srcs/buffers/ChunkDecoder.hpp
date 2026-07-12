#pragma once

#include "InBuffer.hpp"
#include "StrView.hpp"
#include <cstddef>

/* Strips chunked Transfer-Encoding framing.
 * Owns an InBuffer for raw socket reads.
 * readIn() fills the input buffer.
 * decodeAndFlush() parses hex sizes and CRLFs,
 * writes only payload bytes to outFd.
 * State machine suspends/resumes across segments
 * and epoll events when chunk boundaries straddle reads. */
class ChunkDecoder
{
public:
	ChunkDecoder(int inFd, int outFd, BufferManager &bm,
				 const StrView &leftover);

	StrView readIn();
	ssize_t decodeAndFlush();
	bool done() const;

private:
	ChunkDecoder();
	ChunkDecoder(const ChunkDecoder &other);
	ChunkDecoder &operator=(const ChunkDecoder &other);

	InBuffer _input;
	const int _outFd;

	enum State { PARSE_SIZE, SIZE_LF, PAYLOAD, TRAIL_CR, TRAIL_LF, FINISH };
	State _state;

	char _hexBuf[16];
	size_t _hexLen;
	size_t _payloadRemaining;
	size_t _cursor;

	void popIfConsumed();
	bool parseHexDigits();
	bool expectChar(char expected);
	bool writePayload(ssize_t &total);
};
