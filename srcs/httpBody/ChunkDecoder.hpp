#pragma once
#include "Hex.hpp"
#include "ScatterIOBuffer.hpp"
#include <sys/types.h>

/* De-frames HTTP chunked Transfer-Encoding.
 * readIn() inherited fills segments from the socket.
 * writeOut() parses framing in place and gathers
 * payload spans into one scatter write per epoll wake.
 * The held segment survives several wakes — one segment packs many chunks.
 * Size lines are parsed byte-by-byte into _sizeScratch;
 * _expectLf carries a pending CR across a segment boundary.
 * Malformed framing throws 400. */
class ChunkDecoder : public ScatterIOBuffer
{
private:
	/* Constructors */
	ChunkDecoder();
	ChunkDecoder(const ChunkDecoder &other);
	ChunkDecoder &operator=(const ChunkDecoder &other);

	enum e_state { SIZE, DATA, DATA_TRAIL, FINISH };
	static const size_t SCRATCH_SIZE = 16;

protected:
	/* State */
	e_state _state;
	size_t _remainingChunkLen; // payload bytes left in current chunk
	size_t _cursor;			   // parse offset inside _curSegment
	char _sizeScratch[SCRATCH_SIZE];
	size_t _scratchLen;
	bool _expectLf;
	Hex _hex;

	/* Methods */
	void loadSectionsToSend();
	bool parseHexStr(const StrView &view);
	void takePayload(const StrView &view);
	void skipTrailer(const StrView &view);
	bool commitSize();
	bool takeSegment();

	// Held across wakes — one segment packs many chunks. Spent at its end.
	virtual bool segmentSpent() const;

public:
	/* Constructors */
	ChunkDecoder(const Reader &reader, const Writer &writer,
				 BufferManager &pool);
	virtual ~ChunkDecoder();

	/* Methods */
	virtual ssize_t writeOut();
	virtual bool done() const;
};
