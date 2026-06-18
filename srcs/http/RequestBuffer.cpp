#include "RequestBuffer.hpp"
#include "Logger.hpp"
#include "StrView.hpp"
#include "exception/Exception.hpp"
#include "webServ.hpp"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

using std::runtime_error;
using std::string;

// Public constructors and destructors
RequestBuffer::RequestBuffer(int fd) :
	_fd(fd),
	_recvOffset(0),
	_parsingOffset(0) {
	_buff.reserve(RECV_SIZE * 5);
}

RequestBuffer::~RequestBuffer() {}

// Public Methods
string &RequestBuffer::getBuffRef() { return _buff; }
char *RequestBuffer::getParsingPtr() { return &_buff[_parsingOffset]; }

bool RequestBuffer::recvAppend(uint fd) {
	size_t offset = _buff.size();

	char cStrBuffer[RECV_SIZE + 1];
	ssize_t bytesRead = recv(fd, cStrBuffer, RECV_SIZE, 0);

	if (bytesRead <= ERR) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return false;

		char msg[] = "recv() failure reading from client";
		throw(runtime_error(TRACED(msg) + string(strerror(errno))));
	}
	if (bytesRead == 0)
		throw ClientClosed();

	cStrBuffer[bytesRead] = '\0';
	_buff.append(cStrBuffer);
	_curRead = StrView(_buff.c_str() + offset, bytesRead);

	LOG_OBJ("_curRead: ", _curRead);
	LOG_LABELED(Logger::CONTENT, "RECV buffer: ", _buff.c_str());
	return true;
}
