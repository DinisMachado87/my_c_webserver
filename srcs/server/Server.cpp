#include "Server.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "Overrides.hpp"
#include "webServ.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <unistd.h>
#include <vector>

using std::ostream;
using std::string;
using std::stringstream;
using std::vector;

// Public constructors and destructors
Server::Server(const Location &programDefaults) :
	_programDefaults(programDefaults),
	_defaultLocation(_strvVecBuf),
	_defaults(_strvVecBuf) {
	_strBuf.append(DEFAULT_ROOT DEFAULT_INDEX);
};

Server::~Server() {}

// Listen Struct private methods
in_addr_t Listen::getHost() const { return _host; }
uint16_t Listen::getPort() const { return _port; }

// Private Methods
size_t Server::getLoncationsLen() { return _locations.size(); }
size_t Server::getListenLen() { return _listen.size(); }

void Server::reserve(uint sizeStrBuf, uint sizeStrvVecBuf, uint sizeintVecBuf) {
	_strBuf.reserve(sizeStrBuf);
	_strvVecBuf.reserve(sizeStrvVecBuf);
	_intVecBuf.reserve(sizeintVecBuf);
}

const char *Server::safeStr(const char *str) const {
	return str ? str : "NULL";
}

string Server::formatIP(in_addr_t addr) const {
	struct in_addr in;
	in.s_addr = addr;
	return std::string(inet_ntoa(in));
}

// getters
const Location &Server::findLocation(const StrView &path) const {
	const uint pathLen = path.size();

	vector<Location>::const_iterator cur = _locations.begin();
	vector<Location>::const_iterator end = _locations.end();
	for (; cur != end; ++cur)
		if (path.ncompare(cur->getPath(), pathLen)) {
			LOG_OBJ_FUNC("Found Location: ", &(*cur), &Location::printLocation);
			return (*cur);
		}

	LOG_OBJ_FUNC("location not found. Returning default Location: ",
				 &_defaultLocation, &Location::printLocation);
	return _defaultLocation;
};

bool Server::isAllowedMethod(uchar method, const Location &location) const {
	if (!location.usingDefaultMethods() && location.isAllowedMethod(method))
		return true;
	if (!_defaultLocation.usingDefaultMethods()
		&& _defaultLocation.isAllowedMethod(method))
		return true;
	if (_programDefaults.isAllowedMethod(method))
		return true;
	return false;
}

// Print, debug and Logging

void Server::getServerStr(ostream &stream) const {
	if (!LOGGING)
		return;

	stream << "----- SERVER -----" << '\n';
	stream << "\nListen addresses :" << '\n';
	for (size_t i = 0; i < _listen.size(); i++) {
		stream << "  [" << i << "] Host: " << formatIP(_listen[i].getHost())
			   << ", Port: " << _listen[i].getPort() << '\n';
	}
	stream << '\n';

	_defaultLocation.printLocation(DEFAULT_LOCATION, stream);

	stream << "\nLocations: " << '\n';
	for (size_t i = 0; i < _locations.size(); i++)
		_locations[i].printLocation(i, stream);
	stream << "-----" << '\n';
}

void Server::printBufferSizes(ostream &stream) const {
	stream << "\nBuffer Sizes:" << '\n';
	stream << "  String buffer: " << _strBuf.size() << "/" << _strBuf.capacity()
		   << '\n';
	stream << "  StrView buffer: " << _strvVecBuf.size() << "/"
		   << _strvVecBuf.capacity() << '\n';
	stream << "  Int buffer: " << _intVecBuf.size() << "/"
		   << _intVecBuf.capacity() << '\n';
}

std::ostream &operator<<(std::ostream &stream, const Server &server) {
	server.getServerStr(stream);
	return (stream);
}
