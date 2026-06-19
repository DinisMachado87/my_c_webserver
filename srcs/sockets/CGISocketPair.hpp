#include "BufferManager.hpp"
#include "CGISocketPair.hpp"
#include "Logger.hpp"
#include "Response.hpp"
#include <stdexcept>
#include <unistd.h>

using std::runtime_error;

// Public constructors and destructors
CGISocketPair::CGISocketPair(const int fd, const Server &server,
							 struct sockaddr_in serverAddr, Response &response,
							 BufferManager &bufferManager) :
	ASocket(fd, server, serverAddr, bufferManager),
	_response(response),
	_inOpen(true),
	_outOpen(true) {}

CGISocketPair::~CGISocketPair() {
	LOGSOCK(Logger::LOG, "Destroying Connection Socket", _fd);
}

// Public Methods
ASocket *CGISocketPair::handleIn() {
	LOGSOCK(Logger::LOG, "CGISocketPair Handel in", _fd);
	try {
		// if (DONE == _response.readFromCGI())
		_inOpen = false;
	} catch (runtime_error err) {
		LOG_ERROR(runtime_error(TRACED(err.what())));
		throw;
	}
	return NULL;
}

void CGISocketPair::handleOut() {
	LOGSOCK(Logger::LOG, "cgiSocketPair Handel out", _fd);
	try {
		// if (DONE == _response.writeToCGI())
		_outOpen = false;
	} catch (const runtime_error &err) {
		LOG_ERROR(err);
		throw;
	}
}

uint32_t CGISocketPair::getEventsNextLoop() {
	uint32_t events = 0;
	if (_outOpen)
		events |= EPOLLOUT;
	if (_inOpen)
		events |= EPOLLIN;
	return events;
}
