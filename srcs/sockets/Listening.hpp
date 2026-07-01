#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP
// Base class
#include "ASocket.hpp"
// Other classes within the project
#include "Connection.hpp"
#include "Server.hpp"
// Libs
#include <cstring>
#include <stdint.h>
// Error handling
#include <cerrno>
#include <stdexcept>
// Networking
#include <netinet/in.h>
#include <sys/socket.h>

#define OK 0
#define BACKLOG_SIZE 128
#define DEFAULT_PROTOCOL 0

/* Passive socket that binds, listens, and accepts new connections.
 * Created via static create() for validation — enforced by private constructor
 */
class Listening : public ASocket
{
private:
	/* Private Constructor */
	Listening(const int fd, const Server &server, struct sockaddr_in serverAddr,
			  BufferManager &bufferManager);
	/* Explicit disables */
	Listening();
	Listening(const Listening &other);
	Listening &operator=(const Listening &other);
	// Error Handeling
	static std::runtime_error handleFdError(const char *errMsg,
											const int fdSock);

public:
	~Listening();

	// handleIn() accepts a new client and returns a Connection.
	Connection *handleIn();

	// Factory: creates socket. Only access to private constructor.
	static Listening *create(const Server &server, const Listen &listenSock,
							 BufferManager &bufferManager);
};

#endif
