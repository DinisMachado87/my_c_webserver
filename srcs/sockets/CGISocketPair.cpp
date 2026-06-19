#ifndef CGISOCKETPAIR_HPP
#define CGISOCKETPAIR_HPP

#include "ASocket.hpp"
#include "BufferManager.hpp"
#include "webServ.hpp"

class CGISocketPair : public ASocket {
private:
	// Explicit disables
	CGISocketPair();
	CGISocketPair(const CGISocketPair &other);
	CGISocketPair &operator=(const CGISocketPair &other);

protected:
	Response &_response;
	uchar _state;
	bool _inOpen;
	bool _outOpen;

public:
	// Constructors and destructors
	CGISocketPair(const int fd, const Server &server,
				  struct sockaddr_in serverAddr, Response &response,
				  BufferManager &bufferManager);
	~CGISocketPair();
	// Operators overload
	// I/O
	ASocket *handleIn();
	void handleOut();
	// methods
	uint32_t getEventsNextLoop();
};

#endif
