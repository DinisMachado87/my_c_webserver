#include "Engine.hpp"
#include "ASocket.hpp"
#include "ConfParser.hpp"
#include "Connection.hpp"
#include "Listening.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "exception/Exception.hpp"
#include "webServ.hpp"
#include <bits/types/error_t.h>
#include <cerrno>
#include <cstring>
#include <map>
#include <netinet/in.h>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include <utility>
#include <vector>

using std::map;
using std::runtime_error;
using std::string;
using std::stringstream;
using std::vector;

// Public constructors and destructors
Engine::Engine() :
	_fdEpoll(-1) {}

Engine::~Engine() {
	map<int, ASocket *>::iterator socket = _sockets.begin();
	while (socket != _sockets.end())
		delete (socket++)->second;

	vector<Server *>::iterator server = _servers.begin();
	while (server != _servers.end())
		delete *server++;

	if (_fdEpoll > 0)
		close(_fdEpoll);
}

// Error handeling
runtime_error Engine::handleError(const string errMsg, const int err) {
	return runtime_error(errMsg + " :" + strerror(err));
}

// Public Methods
void Engine::epoll_init() {
	_fdEpoll = epoll_create(1);
	if (_fdEpoll < 0)
		throw handleError("Error Epoll_create: ", errno);
}

ASocket *Engine::getSocket(int fd) {
	map<int, ASocket *>::iterator socket = _sockets.find(fd);
	if (socket != _sockets.end())
		return socket->second;
	return NULL;
}

void Engine::setEventTo(int epollFd, uint operation, uint eventType,
						int socketFd, ASocket *socket) {
	struct epoll_event event;
	event.events = eventType;
	event.data.ptr = socket;
	if (OK == epoll_ctl(epollFd, operation, socketFd, &event))
		return;
	throw handleError(TRACED("Epool_ctr"), errno);
}

void Engine::addSocket(ASocket *socket) {
	if (!socket)
		throw handleError("Error null socket", errno);

	int fd = socket->getFd();
	if (!socket || fd < 0)
		throw handleError("Error adding socket", errno);

	_sockets[fd] = socket;
	setEventTo(_fdEpoll, EPOLL_CTL_ADD, socket->trackCurEvents(EPOLLIN), fd,
			   socket);
}

void Engine::deleteSocket(ASocket *socket) {
	int fd = socket->getFd();
	try {
		setEventTo(_fdEpoll, EPOLL_CTL_DEL, 0, fd, socket);
		_sockets.erase(fd);
		delete socket;
		LOGSOCK(Logger::LOG, "Deleted socket", fd);
	} catch (runtime_error &err) {
		throw runtime_error(TRACED(err.what()));
	}
}

void Engine::buildServers(string &config) {
	ConfParser parser(config, _servers, _defaultsVecBuf);
	parser.createServers();
}

void Engine::createSockets() {
	vector<Server *>::iterator server = _servers.begin();

	for (; server != _servers.end(); ++server) {
		const vector<Listen> &listen = (*server)->getListen();
		vector<Listen>::const_iterator port = listen.begin();

		for (; port != listen.end(); ++port) {
			Listening *socket = Listening::create(**server, *port);
			addSocket(socket);
		}
	}
}

void Engine::logFlagUpdates(ASocket *socket, uint32_t events,
							uint32_t newEvents) {
	stringstream stream;
	bool newEventsIn = newEvents & EPOLLIN;
	bool newEventsOut = newEvents & EPOLLOUT;
	bool EventsIn = events & EPOLLIN;
	bool EventsOut = events & EPOLLOUT;
	if (!newEventsIn && EventsIn)
		LOGSOCK(Logger::LOG, "Removing EPOLLIN\n", socket->getFd());
	else if (newEventsIn && !EventsIn)
		LOGSOCK(Logger::LOG, "Adding EPOLLIN\n", socket->getFd());
	if (!newEventsOut && EventsOut)
		LOGSOCK(Logger::LOG, "Removing EPOLLOUT\n", socket->getFd());
	else if (newEventsOut && !EventsOut)
		LOGSOCK(Logger::LOG, "Adding EPOLLOUT\n", socket->getFd());
}

void Engine::updateFlags(ASocket *socket) {
	uint32_t events = socket->getCurEvents();
	uint32_t newEvents = socket->getEventsNextLoop();

	LOGEVENTS(socket, events, newEvents);
	if (newEvents != events) {
		socket->trackCurEvents(newEvents);
		setEventTo(_fdEpoll, EPOLL_CTL_MOD, newEvents, socket->getFd(), socket);
	}
}

void Engine::pollLoop() {
	struct epoll_event events[MAX_EVENTS];
	int nFds = -1;

	while (!g_shutdown) {
		nFds = -1;
		usleep(100000);
		LOG_TITLE("New Loop");
		nFds = epoll_wait(_fdEpoll, events, MAX_EVENTS, TIMEOUT);
		if (ERR == nFds) {
			if (errno == EINTR)
				continue;
			throw handleError(TRACED("Epoll_wait: "), errno);
		}

		for (int i = 0; i < nFds && !g_shutdown; i++) {
			LOG_TITLE("Next socket");
			ASocket *socket = static_cast<ASocket *>(events[i].data.ptr);
			uint32_t ev = events[i].events;
			try {
				if (ev & EPOLLERR)
					throw runtime_error(TRACED("EpollWait")
										+ string(strerror(errno)));
				if (ev & EPOLLHUP)
					throw ClientClosed();
				if (ev & EPOLLIN)
					while (ASocket *connection = socket->handleIn())
						if (connection)
							addSocket(connection);
				if (ev & EPOLLOUT) {
					LOG(Logger::LOG, "Received EPOLLOUT");
					socket->handleOut();
				}
				updateFlags(socket);
			} catch (const ClientClosed &ex) {
				LOGSOCK(Logger::LOG, ex.what(), socket->getFd());
				deleteSocket(socket);
			} catch (const runtime_error &err) {
				LOG_ERROR_LABELED("Handeling socket event", err);
				deleteSocket(socket);
			}
		}
	}
}

void Engine::run(string &config) {
	try {
		epoll_init();
		buildServers(config);
		createSockets();
		pollLoop();
	} catch (runtime_error err) {
		LOG_ERROR(err);
	}
}
