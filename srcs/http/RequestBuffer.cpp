#include "RequestBuffer.hpp"
#include "BufferManager.hpp"
#include "Logger.hpp"
#include "StrView.hpp"
#include "exception/Exception.hpp"
#include "webServ.hpp"
#include <cerrno>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

using std::string;

// Public constructors and destructors
RequestBuffer::RequestBuffer(int fd, BufferManager &bufferManager) :
	_bufferManager(bufferManager),
	_fd(fd) {}

RequestBuffer::~RequestBuffer() { _bufferManager.returnBuffers(_buffers); }

// Public Methods
StrView RequestBuffer::getlastRead() { return _lastRead; }

StrView RequestBuffer::recvAppend() {
	char *buffer = _bufferManager.getBuffer();
	if (!buffer)
		return StrView();

	ssize_t bytesRead = recv(_fd, buffer, RECV_SIZE - 1, 0);
	if (bytesRead <= ERR)
		// Allways returns as EGAIN EWOULDBLOCK.
		// Actual Errors handeled through EPOOLERROR.
		return StrView();
	if (bytesRead == 0)
		throw ClientClosed();
	buffer[bytesRead] = '\0';
	_lastRead = StrView(buffer, bytesRead);
	_buffers.push_back(_lastRead);

	LOG_OBJ("_lastRead: ", _lastRead);
	return _lastRead;
}
