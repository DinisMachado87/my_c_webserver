#ifndef CGISOCKETPAIR_HPP
#define CGISOCKETPAIR_HPP

#include "ASocket.hpp"
#include "BufferManager.hpp"
#include "webServ.hpp"

/* Bidirectional pipe to a CGI child process.
 * Tracks read/write halves independently — each half
 * closes when its transfer is done. */
class CGISocketPair : public ASocket
{
private:
	/* Explicit disables */
	CGISocketPair();
	CGISocketPair(const CGISocketPair &other);
	CGISocketPair &operator=(const CGISocketPair &other);

protected:
	Response &_response;
	uchar _state;
	bool _inOpen;  // still reading from CGI child
	bool _outOpen; // still writing to CGI child

public:
	CGISocketPair(const int fd, const Server &server,
				  struct sockaddr_in serverAddr, Response &response,
				  BufferManager &bufferManager);
	~CGISocketPair();

	/* I/O */
	ASocket *handleIn();
	void handleOut();

	// Returns EPOLLIN | EPOLLOUT to be set for the next loop.
	uint32_t getEventsNextLoop();
};

#endif
