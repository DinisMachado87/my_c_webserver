#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "ASocket.hpp"
#include "HttpParser.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include <cstddef>
#include <sys/epoll.h>

/* Client connection. Holds the single Response in flight — the next
 * request is not parsed until the current response is fully sent. */
class Connection : public ASocket
{
private:
	HttpParser _http;
	Response *_response;

	enum e_handleInState {
		REQUEST,
		INITBODY,
		LOOPBODY,
		DONE_READING
	} _handleInState;

	/* Explicit disables */
	Connection(const Connection &other);
	Connection &operator=(const Connection &other);

	/* Constructor for friend */
	// Only Listening can construct — accepts the fd from accept().
	Connection(const int fd, const Server &server,
			   struct sockaddr_in serverAddr, BufferManager &bufferManager);
	friend class Listening;

public:
	~Connection();
	/* I/O */
	ASocket *handleIn();
	void handleOut();
	/* Epoll */
	uint32_t getEventsNextLoop();
};

#endif
