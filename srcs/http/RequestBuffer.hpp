#ifndef REQUESTBUFFER_HPP
#define REQUESTBUFFER_HPP

#include "StrView.hpp"
#include <string>
#include <sys/types.h>
#include <unistd.h>

class RequestBuffer {
private:
	std::string _buff;
	const int _fd;

	size_t _recvOffset;
	size_t _parsingOffset;

	StrView _curRead;

	// Explicit disables
	RequestBuffer();
	RequestBuffer(const RequestBuffer &other);
	RequestBuffer &operator=(const RequestBuffer &other);

protected:
	friend class Request;
	friend class HttpParser;
	RequestBuffer(int fd);
	std::string &getBuffRef();

public:
	// Constructors and destructors
	~RequestBuffer();

	// Getters and setters
	char *getCur();

	// Methods
	bool recvAppend(uint fd);
	char *getParsingPtr();
};

#endif
