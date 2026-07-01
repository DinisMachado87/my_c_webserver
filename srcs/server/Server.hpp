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

class Listen
{
private:
	in_addr_t _host;
	uint16_t _port;
	friend class ConfParser;
	friend class ConfParserTest;

public:
	uint16_t getPort() const;
	in_addr_t getHost() const;
};

/* Parsed config for one server block. Owns contiguous memory for all
 * StrViews (_strBuf) and Spans (_strvVecBuf) so lookups at runtime
 * hit a single allocation. */
class Server
{
private:
	/* Contiguous StrView buffers*/
	std::string _strBuf;			  // single buffer for all strviews
	std::vector<StrView> _strvVecBuf; // single buffer for all spans

	/* Server Components*/
	Location _defaults; // server-level overrides, used as fallback
	std::vector<Listen> _listen;
	std::vector<Location> _locations;

	/* Explicit disables*/
	Server &operator=(const Server &other);
	Server(const Server &other);

	std::string formatIP(in_addr_t addr) const;
	void printBufferSizes(std::ostream &stream) const;

	friend class ConfParser;
	friend class ConfParserTest;

public:
	Server();
	~Server();

	// Returns matching location, or _defaults if none matches.
	const Location &findLocation(const StrView &path) const;

	/* Listen accessors */
	const std::vector<Listen> &getListen() const;
	size_t getListenLen() const;

	void getServerStr(std::ostream &stream) const;
};

std::ostream &operator<<(std::ostream &stream, const Server &server);
