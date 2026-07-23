#pragma once
#include "Hex.hpp"
#include "IOBuffer.hpp"
#include "ScatterList.hpp"
#include <sys/types.h>

/* Frames outgoing data as HTTP chunk [hex][\r\n][data][\r\n]
 * through one scatter write per epoll wake.
 * Partial sends resume - _curSegment stays held until the batch drains.
 * On EOF arms the "0\r\n\r\n" terminator */
class ChunkEncoder : public IOBuffer
{
private:
	/* Constructors */
	ChunkEncoder();
	ChunkEncoder(const ChunkEncoder &other);
	ChunkEncoder &operator=(const ChunkEncoder &other);

	enum e_state { FRAME, SEND, SEND_TERMINATOR, FINISH };
	static const int MAX_SECTIONS = 4;

protected:
	/* State */
	e_state _state;
	Hex _hex;
	Segment *_curSegment;
	ScatterList<MAX_SECTIONS> _iovSegments;

	/* Methods */
	ssize_t frameAndSend();
	ssize_t resumeSend(e_state nextState);
	bool frameSegment();
	void frameTerminator();

public:
	/* Constructors */
	ChunkEncoder(int inFd, Reader::FdType inFdType, int outFd,
				 e_outFdType outFdType, BufferManager &pool);
	virtual ~ChunkEncoder();

	/* Methods */
	virtual ssize_t writeOut();
	virtual bool done() const;
};
