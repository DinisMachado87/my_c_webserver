#pragma once

#include "BufferManager.hpp"
#include "HttpParser.hpp"
#include <netinet/in.h>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <unistd.h>

class Server;
class Connection;

/* Abstract base for epoll-managed sockets.
 * Subclasses: Listening (accept), Connection (client I/O),
 * CGISocketPair (CGI child I/O). */
class ASocket
{
public:
	virtual ~ASocket();

	/* I/O — subclasses define the actual read/write logic */
	virtual ASocket *handleIn() = 0;
	virtual void handleOut();

	/* Epoll event management */
	virtual uint32_t getEventsNextLoop();
	uint32_t getCurEvents() const;
	uint32_t trackCurEvents(uint32_t events);

	bool isFull();
	int getFd() const;

	static int setNonBlocking(int fd);

private:
	/* Explicit disables*/
	ASocket();
	ASocket(const ASocket &other);
	ASocket &operator=(const ASocket &other);

protected:
	void *_ptrToSelf;
	int _fd;
	const Server &_server;
	struct sockaddr_in _serverAddr;
	HttpParser _parser;
	Request *_request;
	uint32_t _events; // last epoll events assigned
	BufferManager &_bufferManager;
	// Constructors and destructors
	ASocket(const int fd, const Server &server, struct sockaddr_in serverAddr,
			BufferManager &bufferManager);
	// Error Handeling
	static std::runtime_error handleError(const std::string errMsg);
};
