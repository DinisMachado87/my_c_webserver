#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <exception>

class ClientClosed : public std::exception {
public:
	ClientClosed() {}
	~ClientClosed() throw() {}

	// Methods
	const char *what() const throw() { return "Client closed connection"; }
};

#endif
