#include "Connection.hpp"
#include "ASocket.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
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
					   struct sockaddr_in serverAddr) :
	ASocket(fd, server, serverAddr),
	_http(server, fd),
	_responseReceivingBody(NULL),
	_cur(0),
	_back(0),
	_handleInState(REQUEST) {
	for (size_t i = 0; i < RESPONSES_CUE_SIZE; i++)
		_responses[i] = NULL;
}

Connection::~Connection() {
	LOGSOCK(Logger::LOG, "Destroying Connection Socket", _fd);
	while (_responses[_cur]) {
		_responses[_cur] = NULL;
		_cur = (_cur + 1) % RESPONSES_CUE_SIZE;
	}
}

// Public Methods
ASocket *Connection::handleIn() {
	LOGSOCK(Logger::LOG, "Connection Handel in", _fd);

	Request *request = NULL;

	try {
		switch (_handleInState) {
		case REQUEST:
			request = _http.recvAndParse();
			if (!request)
				return NULL;

			_responseReceivingBody = _responses[_back];
			_back = ((_back + 1) % RESPONSES_CUE_SIZE);
			LOGSOCKNUM(Logger::LOG, "Stored _response on slot ", _back, _fd);

		case INITBODY: // fallthrough
					   // _handleInState = readBody(true, "[INITBODY]", buffer,
					   // bytesRead);
		default:	   // fallthrough
			return NULL;
		}

	} catch (runtime_error err) {
		LOG_ERROR(runtime_error(TRACED(err.what())));
		throw;
	}
}

void Connection::handleOut() {
	LOGSOCK(Logger::LOG, "Connection Handel out", _fd);
	try {
		if (!_responses[_cur]) {
			LOGSOCK(Logger::WARNING, "handleOut called without response", _fd);
			return;
		}

		// bool state = _responses[_cur]->sendResponse(_fd);
		// LOGSOCK_LABELED(Logger::LOG, "SENT response status ",
		// 				(state ? "DONE" : "ONGOING"), _fd);
		// if (DONE == state) {
		// 	LOGSOCKNUM(Logger::LOG, "DONE: Deleting response idx: ", _cur, _fd);
		// 	delete _responses[_cur];
		// 	_responses[_cur] = NULL;
		// 	_cur = (_cur + 1) % RESPONSES_CUE_SIZE;
		// }
	} catch (const runtime_error &err) {
		LOG_ERROR(err);
		throw;
	}
}

uint32_t Connection::getEventsNextLoop() {
	uint32_t events = 0;
	LOGSOCKNUMS2(Logger::LOG, "_responses[i] next Loop cur/back: ", _cur, _back,
				 _fd);
	if (_responses[_cur])
		events |= EPOLLOUT;
	if (!isFull())
		events |= EPOLLIN;
	return events;
}

bool Connection::isFull() const {
	int next = (_cur + 1) % RESPONSES_CUE_SIZE;
	return (_responses[_cur] && _responses[next]);
}
