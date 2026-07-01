#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "ASocket.hpp"
#include "BufferManager.hpp"
#include "Server.hpp"
#include "webServ.hpp"
#include <map>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <vector>

/* Higher class.
 * Manages Sockets epool.
 * Owns servers, sockets, the epoll fd,
 * 	and the shared resorces that need to persist beyond socket lifetime
 * like BufferManager and Cookies.  */
class Engine
{
private:
	/* Explicit Disables*/
	Engine(const Engine &other);
	Engine &operator=(const Engine &other);

protected:
	int _fdEpoll;
	std::vector<Server *> _servers;
	std::map<int, ASocket *> _sockets; // fd -> socket, owns the pointers
	BufferManager _bufferManager;

	std::vector<StrView> _defaultsVecBuf;

	/* Setup */
	void epoll_init();
	void buildServers(std::string &config);
	void createSockets(); // one Listening per listen directive

	/* Socket management */
	void addSocket(ASocket *socket);	// registers in map + epoll
	void deleteSocket(ASocket *socket); // removes from epoll, map, and deletes
	ASocket *getSocket(int fd);
	void setEventTo(int epollFd, uint operation, uint eventType, int socketFd,
					ASocket *ptrToSock);

	/* Event loop */
	void pollLoop();
	void updateFlags(ASocket *socket); // syncs epoll interest with socket state

	void logFlagUpdates(ASocket *socket, uint32_t events, uint32_t newEvents);
	std::runtime_error handleError(const std::string errMsg, const int err);

public:
	Engine();
	~Engine(); // closes epoll fd, deletes all sockets and servers

	// Entry point - Parses config, creates listening sockets, enters poll loop.
	void run(std::string &config);
};

#ifdef LOGGING
#define LOGEVENTS(socket, events, newEvents) \
	Engine::logFlagUpdates(socket, events, newEvents)
#else
#define LOGEVENTS(socket, events, newEvents) (void)0
#endif

#endif
