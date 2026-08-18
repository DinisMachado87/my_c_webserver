#include "ScatterIOBuffer.hpp"
#include "BufferManager.hpp"
#include "Reader.hpp"
#include "Writer.hpp"

ScatterIOBuffer::ScatterIOBuffer(const Reader &reader, const Writer &writer,
								 BufferManager &pool) :
	IOBuffer(reader, writer, pool)
{
}

ScatterIOBuffer::~ScatterIOBuffer() {}

/* Protected Methods */
ssize_t ScatterIOBuffer::sendSections()
{
	if (!_iovSections.loadedSections())
		return 0;

	ssize_t sent = _writer.writeGather(_iovSections.iov(),
									   _iovSections.loadedSections());
	if (sent > 0)
		_iovSections.advance(sent);
	return sent;
}
