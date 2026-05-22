#pragma once

#include "Location.hpp"
#include "Overrides.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <netinet/in.h>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <vector>

struct Listen {
	in_addr_t _host;
	uint16_t _port;
	// Methods
	uint16_t getPort() const;
	in_addr_t getHost() const;
};

class Server {
private:
	Location _defaultLocation;

	// Explicit Disables
	Server &operator=(const Server &other);
	// Implemented for friend classes
	Server(const Server &other);

	friend class ConfParser;
	friend class ConfParserTest;

protected:
	// Contiguous Buffers
	std::string _strBuf;
	std::vector<StrView> _strvVecBuf;
	std::vector<uint> _intVecBuf;
	// Private methods
	std::string formatIP(in_addr_t addr) const;
	void printBufferSizes(std::ostream &stream) const;
	void printLocation(const Location &loc, size_t index,
					   std::stringstream &stream) const;
	void printOverrides(const Overrides &over, const char *label,
						std::stringstream &stream) const;
	const char *safeStr(const char *str) const;

public:
	Overrides _defaults;
	std::vector<Listen> _listen;
	std::vector<Location> _locations;
	// Constructors and destructors
	Server();
	~Server();

	// Methods
	void reserve(uint sizeStrBuf, uint sizeStrvVecBuf, uint sizeintVecBuf);
	// Getters Server Vars
	size_t getListenLen();
	size_t getLoncationsLen();
	const Location &findLocation(const StrView &path) const;
	void getServerStr(std::ostream &stream) const;
};

std::ostream &operator<<(std::ostream &stream, const Server &server);
