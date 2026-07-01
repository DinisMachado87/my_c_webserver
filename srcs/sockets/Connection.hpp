#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "ASocket.hpp"
#include "HttpParser.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <sys/epoll.h>

/* Client connection. Owns a fixed-size ring buffer of Responses
 * so pipelined requests can be parsed while earlier responses
 * are still being sent. */
class Connection : public ASocket
{
private:
	enum _handleInState { REQUEST, RESPONSE, INITBODY, LOOPBODY };
	HttpParser _http;
	Response *_responses[RESPONSES_CUE_SIZE]; // ring buffer
	Response *_responseReceivingBody;		  // slot currently receiving body
	size_t _cur;							  // oldest unsent response
	size_t _back;							  // next free slot

	uchar _handleInState;

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
	bool isFull() const;
	size_t recvToBuffer(char *buffer);
};

#endif
