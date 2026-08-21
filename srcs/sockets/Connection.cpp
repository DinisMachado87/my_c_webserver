#include "Connection.hpp"
#include "ASocket.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Traced.hpp"
#include "webServ.hpp"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

using std::ifstream;
using std::runtime_error;
using std::string;

// Public constructors and destructors
Connection::Connection(const int fd, const Server &server,
					   struct sockaddr_in serverAddr,
					   BufferManager &bufferManager) :
	ASocket(fd, server, serverAddr, bufferManager),
	_http(server, fd, _bufferManager),
	_response(NULL),
	_handleInState(REQUEST)
{
}

Connection::~Connection()
{
	LOGSOCK(Logger::LOG, "Destroying Connection Socket", _fd);
	delete _response;
}

// Public Methods
ASocket *Connection::handleIn()
{
	LOGSOCK(Logger::LOG, "Connection Handel in", _fd);
	try {
		switch (_handleInState) {
		case REQUEST:
			_response = _http.recvAndParse();
			if (!_response)
				return NULL;
			LOGSOCK(Logger::LOG, "Received _response", _fd);
			// fallthrough

		case INITBODY:
			// _handleInState = readBody(true, "[INITBODY]", buffer,
			// bytesRead);

		default:
			return NULL;
		}

	} catch (const runtime_error &err) {
		LOG_ERROR(runtime_error(TRACED(err.what())));
		throw;
	}
}

void Connection::handleOut()
{
	LOGSOCK(Logger::LOG, "Connection Handel out", _fd);
	try {
		if (!_response) {
			LOGSOCK(Logger::WARNING, "handleOut called without response", _fd);
			return;
		}

		if (DONE == _response->send()) {
			LOGSOCK(Logger::LOG, "DONE: Deleting response", _fd);
			delete _response;
			_response = NULL;
			_handleInState = REQUEST;
		}
	} catch (const runtime_error &err) {
		LOG_ERROR(err);
		throw;
	}
}

uint32_t Connection::getEventsNextLoop()
{
	uint32_t events = 0;
	if (_response)
		events |= EPOLLOUT;
	if (_handleInState != DONE_READING)
		events |= EPOLLIN;
	return events;
}
