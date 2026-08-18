#pragma once
#include "Hex.hpp"
#include "ScatterIOBuffer.hpp"
#include <sys/types.h>

/* Frames outgoing data as HTTP chunk [hex][\r\n][data][\r\n]
 * through one scatter write per epoll wake.
 * Partial sends resume — the held segment stays until sections drain.
 * On input EOF arms the "0\r\n\r\n" terminator. */
class ChunkEncoder : public ScatterIOBuffer
{
private:
	/* Constructors */
	ChunkEncoder();
	ChunkEncoder(const ChunkEncoder &other);
	ChunkEncoder &operator=(const ChunkEncoder &other);

	enum e_state { FRAME, SEND, SEND_TERMINATOR, FINISH };

protected:
	/* State */
	e_state _state;
	Hex _hex; // reframe only after sections drain — an iovec points at _str

	/* Methods */
	ssize_t resumeSend(e_state nextState);
	bool frameSegment();
	void frameTerminator();

	// One framed segment is spent the moment its sections drain.
	virtual bool segmentSpent() const;

public:
	/* Constructors */
	ChunkEncoder(const Reader &reader, const Writer &writer,
				 BufferManager &pool);
	virtual ~ChunkEncoder();

	/* Methods */
	virtual ssize_t writeOut();
	virtual bool done() const;
};
