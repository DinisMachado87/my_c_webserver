#pragma once

#include "InOutBuffer.hpp"
#include <cstddef>

/* Frames outgoing data as HTTP chunked Transfer-Encoding.
 * readIn() inherited — fills segments from file/CGI.
 * writeOut() overridden — sends each segment as a chunk
 * using sendmsg() scatter-gather: [hex\r\n][data][\r\n]
 * in one syscall, zero userspace copies.
 * Handles partial sends across epoll events.
 * Call finalize() when input is exhausted — writeOut()
 * will drain queued segments then send the terminator. */
class ChunkEncoder : public InOutBuffer
{
public:
	ChunkEncoder(int inFd, int outFd, BufferManager &bm);

	ssize_t writeOut();
	void finalize();

protected:
	StrView onEof();

private:
	/* Explicit disables */
	ChunkEncoder();
	ChunkEncoder(const ChunkEncoder &other);
	ChunkEncoder &operator=(const ChunkEncoder &other);

	enum State { PREPARE, SEND, SEND_TERMINATOR, FINISH };
	State _state;
	bool _finishing;

	char _sizeHeader[20]; // "hex\r\n" or "0\r\n" in terminator
	size_t _headerLen;
	size_t _payloadLen;
	size_t _sent;

	void makeSizeHeader();
	void makeTerminator();
	bool prepareNextChunk();
	int buildChunk(struct iovec *segmentsVec);
	ssize_t sendChunk(struct iovec *segmentsVec, int nSections);
	void finalizeChunk();
	bool done() const;
};
