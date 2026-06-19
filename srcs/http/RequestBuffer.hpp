#ifndef REQUESTBUFFER_HPP
#define REQUESTBUFFER_HPP

#include "BufferManager.hpp"
#include "StrView.hpp"
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class RequestBuffer {
private:
	BufferManager &_bufferManager;
	std::vector<StrView> _buffers;
	const int _fd;

	StrView _lastRead;
	StrView _freeSpace;
	StrView _leftover;

	// Explicit disables
	RequestBuffer();
	RequestBuffer(const RequestBuffer &other);
	RequestBuffer &operator=(const RequestBuffer &other);

protected:
	friend class Request;
	friend class HttpParser;
	RequestBuffer(int fd, BufferManager &bufferManager);
	std::string &getBuffRef();

public:
	// Constructors and destructors
	~RequestBuffer();

	// Getters and setters
	char *getCur();

	// Methods
	StrView recvAppend();
	StrView getlastRead();
};

#endif
