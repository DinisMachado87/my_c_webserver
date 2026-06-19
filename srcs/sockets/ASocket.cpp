#include "ASocket.hpp"
#include "BufferManager.hpp"
#include "Logger.hpp"
#include "webServ.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>

using std::runtime_error;
using std::string;

// Public constructors and destructors
ASocket::ASocket(const int fd, const Server &server,
				 struct sockaddr_in serverAddr, BufferManager &bufferManager) :
	_fd(fd),
	_server(server),
	_serverAddr(serverAddr),
	_parser(server, fd, bufferManager),
	_events(0),
	_bufferManager(bufferManager) {}

ASocket::~ASocket() {
	if (_fd >= 0)
		close(_fd);
}

// Error handeling
runtime_error ASocket::handleError(const string errMsg) {
	return runtime_error(errMsg + strerror(errno));
}

// Public Methods
int ASocket::getFd() const { return (_fd); }

int ASocket::setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL);
	if (flags == ERR)
		throw runtime_error(TRACED("Getting fcntl flags"));
	if (ERR == fcntl(fd, F_SETFL, flags | O_NONBLOCK))
		throw handleError(TRACED("Setting client sock non-blocking"));
	return OK;
}

uint32_t ASocket::trackCurEvents(uint32_t events) {
	_events = events;
	return _events;
}

// defaults for virtual methods
bool ASocket::isFull() { return false; }
void ASocket::handleOut() {};
uint32_t ASocket::getEventsNextLoop() { return EPOLLIN; }
uint32_t ASocket::getCurEvents() const { return _events; }
