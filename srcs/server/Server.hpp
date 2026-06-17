#pragma once

#include "Location.hpp"
#include "Overrides.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <sys/epoll.h>
#include <vector>

class Listen {
private:
	in_addr_t _host;
	uint16_t _port;
	friend class ConfParser;
	friend class ConfParserTest;

public:
	uint16_t getPort() const;
	in_addr_t getHost() const;
};

class Server {
private:
	// Contiguous StrView buffers
	std::string _strBuf;
	std::vector<StrView> _strvVecBuf;
	std::vector<uint> _intVecBuf;

	// Server Components
	Location _serverDefaults;
	std::vector<Listen> _listen;
	std::vector<Location> _locations;

	// Explicit disables
	Server &operator=(const Server &other);
	Server(const Server &other);

	// Pivate Methods
	void reserve(uint sizeStrBuf, uint sizeStrvVecBuf, uint sizeintVecBuf);
	std::string formatIP(in_addr_t addr) const;
	void printBufferSizes(std::ostream &stream) const;

	friend class ConfParser;
	friend class ConfParserTest;

public:
	Server();
	~Server();

	// Lookup — returns NULL when no location matches the path.
	const Location &findLocation(const StrView &path) const;

	// Resolution (inheritable fields walk up to 3 tiers; pass found loc or
	// NULL).
	uchar resolveMethods(const Location *loc) const;
	bool isAllowedMethod(uchar method, const Location *loc) const;
	const char *resolveRoot(const Location *loc) const;
	const Span<StrView> &resolveIndex(const Location *loc) const;
	bool resolveAutoindex(const Location *loc) const;
	size_t resolveClientMaxBody(const Location *loc) const;
	const char *resolveErrorFile(const Location *loc, uint code) const;

	// Listen accessors
	size_t getListenLen() const;
	const Listen &getListen(size_t i) const;

	void getServerStr(std::ostream &stream) const;
};

std::ostream &operator<<(std::ostream &stream, const Server &server);
