#include "Server.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "Overrides.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <netinet/in.h>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

using std::ostream;
using std::string;
using std::vector;

// Constructors and destructors
Server::Server() :
	_serverDefaults(_strvVecBuf) {}

Server::~Server() {}

// Listen
in_addr_t Listen::getHost() const { return _host; }
uint16_t Listen::getPort() const { return _port; }

const Listen &Server::getListen(size_t i) const { return _listen[i]; }
size_t Server::getListenLen() const { return _listen.size(); }

// Private helpers
void Server::reserve(uint sizeStrBuf, uint sizeStrvVecBuf, uint sizeintVecBuf) {
	_strBuf.reserve(sizeStrBuf);
	_strvVecBuf.reserve(sizeStrvVecBuf);
	_intVecBuf.reserve(sizeintVecBuf);
}

string Server::formatIP(in_addr_t addr) const {
	struct in_addr in;
	in.s_addr = addr;
	return std::string(inet_ntoa(in));
}

// Getters
const Location &Server::findLocation(const StrView &path) const {
	vector<Location>::const_iterator cur = _locations.begin();
	vector<Location>::const_iterator end = _locations.end();
	for (; cur != end; ++cur)
		if (path.compare(cur->getPath(), path.size())) {
			LOG_OBJ_FUNC("Found Location: ", &(*cur), &Location::printLocation);
			return *cur;
		}

	return _serverDefaults;
}

// Print
void Server::getServerStr(ostream &stream) const {
	if (!LOGGING) return;

	stream << "----- SERVER -----\n\nListen addresses:\n";
	for (size_t i = 0; i < _listen.size(); i++)
		stream << "  [" << i << "] Host: " << formatIP(_listen[i].getHost())
			   << ", Port: " << _listen[i].getPort() << '\n';
	stream << '\n';

	_serverDefaults.getOverrides().printOverrides("Server Defaults", stream);

	stream << "\nLocations:\n";
	for (size_t i = 0; i < _locations.size(); i++)
		_locations[i].printLocation(i, stream);
	stream << "-----\n";
}

std::ostream &operator<<(std::ostream &stream, const Server &server) {
	server.getServerStr(stream);
	return stream;
}
