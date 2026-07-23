#pragma once

#include "IBuffer.hpp"
#include "Reader.hpp"
#include "SegmentList.hpp"
#include "Writer.hpp"
#include <sys/types.h>

/* clang-format off */
/*
SEND CASES
| Case                    | Example                              | Class        | fd     | Syscall        |
|-------------------------|--------------------------------------|--------------|--------|----------------|
| Static file verbatim    | plain on-disk GET, known size        | (no Buffer)  | SOCKET | sendfile       |
| Chunked out             | CGI, autoindex, growing file, gzip   | ChunkEncoder | SOCKET | sendmsg        |
| Known-length transformed| SSI/template, in-mem too big for send| IOBuffer     | SOCKET | send           |
| Small in-mem body       | 404/403/500, 30x, 204/304, OPTIONS   | IOBuffer     | SOCKET | send           |
| POST to file, plain     | Content-Length body to disk          | IOBuffer     | FILE   | write          |
| POST to file, chunked   | chunked body deframed to disk        | ChunkDecoder | FILE   | writev         |
| Reject/oversize/decline | 405/413 drain, declined 100-continue | IBuffer      | —      | none (drain)   |
*/
/* clang-format on */

class BufferManager;

/* Input and output buffer. IBuffer's read side plus a Writer — drains the
 * same bytes it read, one segment per writeOut() (level-triggered). NONE
 * out-kind for in-memory bodies: nothing to send, seeded chain drains until
 * done(). ChunkEncoder and ChunkDecoder derive reframing on the way out. */
class IOBuffer : public IBuffer
{
public:
	enum e_outFdType { SOCKET = Writer::SOCKET, FILE = Writer::FILE };
	/* Constructors */
	IOBuffer(const Reader &reader, const Writer &writer, BufferManager &pool);
	virtual ~IOBuffer();

	/* Methods */
	// Sends one segment. Returns bytes sent, 0 if empty, -1 on error.
	virtual ssize_t writeOut();

protected:
	/* State */
	Writer _writer;

private:
	IOBuffer();
	IOBuffer(const IOBuffer &other);
	IOBuffer &operator=(const IOBuffer &other);
};
