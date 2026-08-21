#pragma once

#include "BufferManager.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "StrView.hpp"
#include <string>
#include <sys/stat.h>

/* Validates a parsed Request and produces the correct Response.
 * Called as the final stage of HttpParser's state machine.
 * Throws HttpError on any validation failure — the caller
 * catches and wraps it in an ERRORResponse. */
class Router
{
public:
	/* Single entry point. Constructs a Router on the stack,
	 * resolves everything, returns the Response.
	 * Takes ownership of request (transfers to Response). */
	static Response *route(const Server &server, const int clientFd,
						   BufferManager &bufferManager, Request *request,
						   const StrView leftover);

private:
	const Server &_server;
	BufferManager &_bufferManager;
	const int _clientFd;

	Request *_request;
	const RequestPath &_path;
	const Location &_location;
	const StrView _leftover;

	std::string _absolutePath;

	Router(const Server &server, int clientFd, BufferManager &bufferManager,
		   Request *request, const StrView leftover);
	~Router();

	Response *resolve();

	/* Explicit disables */
	Router();
	Router(const Router &other);
	Router &operator=(const Router &other);

	void statOrThrow(struct stat &state);

	/* Directory handling */
	Response *handleDirectory();
	bool resolveIndexFile(std::string &dest);

	/* Body factory — sets body as ChunkEncoder, ChunkDecoder or IOBuffer */
	IOBuffer *createBody(e_FdType destType, int destFd);

	/* Method dispatch — each returns the appropriate Response subclass */
	Response *routeGET(struct stat &state);
	Response *routePOST();
	Response *routeDELETE();
	Response *routeCGI();
};
