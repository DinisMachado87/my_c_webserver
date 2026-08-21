#include "Router.hpp"
#include "GETResponse.hpp"
#include "HttpError.hpp"
#include "HttpStatus.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "RedirectResponse.hpp"
#include "Request.hpp"
#include "RequestPath.hpp"
#include "Server.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using std::string;

//  Static entry point

Response *Router::route(const Server &server, const int clientFd,
						BufferManager &bufferManager, Request *request,
						const StrView leftover)
{
	Router router(server, clientFd, bufferManager, request, leftover);
	return router.resolve();
}

//  Constructor
Router::Router(const Server &server, int clientFd, BufferManager &bufferManager,
			   Request *request, const StrView leftover) :
	_server(server),
	_bufferManager(bufferManager),
	_clientFd(clientFd),
	_request(request),
	_path(request->getPath()),
	_location(server.findLocation(_path.path())),
	_leftover(leftover)
{
}

Router::~Router() {}

//  Main dispatch
Response *Router::resolve()
{
	LOG_OBJ_FUNC("Resolved _locationtion: ", &_location,
				 &Location::printLocation);

	if (_location.getReturncode()) // true if redirected
		return new RedirectResponse(_request);

	if (!_location.isAllowedMethod(_request->getMethod()))
		throw HttpError(HttpStatus::METHOD_NOT_ALLOWED);

	_absolutePath = _location.getRoot() + _path.path();
	struct stat state;
	statOrThrow(state);

	bool isDir = S_ISDIR(state.st_mode);

	// Directory without trailing slash -> 301 redirect
	if (isDir && _path.getType() != RequestPath::DIR)
		return new RedirectResponse(_request);

	if (isDir)
		return handleDirectory();
	else if (!S_ISREG(state.st_mode)) // is not file or Dir
		throw HttpError(HttpStatus::FORBIDDEN);

	// is Cgi
	const StrView &cgiExt = _path.getCgiExtension();
	if (!cgiExt.empty() && _location.findCgiPath(cgiExt))
		return routeCGI();

	// is not cgi
	switch (_request->getMethod()) {
	case GET:
		return routeGET(state);
	case POST:
		return routePOST();
	case DELETE:
		return routeDELETE();
	default:
		throw HttpError(HttpStatus::METHOD_NOT_ALLOWED);
	}
}

void Router::statOrThrow(struct stat &state)
{
	if (OK == stat(_absolutePath.c_str(), &state))
		return;

	if (errno == ENOENT || errno == ENOTDIR)
		throw HttpError(HttpStatus::NOT_FOUND);
	if (errno == EACCES)
		throw HttpError(HttpStatus::FORBIDDEN);
	throw HttpError(HttpStatus::INTERNAL_SERVER_ERROR);
}

Response *Router::handleDirectory()
{
	// Try index files in order
	string indexPath;
	if (resolveIndexFile(indexPath)) {
		struct stat state;
		_absolutePath = indexPath;
		statOrThrow(state);
		return routeGET(state);
	}

	// Autoindex
	if (_location.getOverrides().isAutoindexed())
		// TODO: return new AutoindexResponse(request, absPath);
		return new Response(HttpStatus::_OK, _request);
	throw HttpError(HttpStatus::FORBIDDEN);
}

bool Router::resolveIndexFile(string &dest)
{
	const Span<StrView> &indexes = _location.getOverrides().getIndex();

	for (size_t i = 0; i < indexes.len(); i++) {
		dest.assign(_absolutePath.data(), _absolutePath.size());
		if (!dest.empty() && dest[dest.size() - 1] != '/')
			dest += '/';
		dest.append(indexes[i].data(), indexes[i].size());

		struct stat state;
		if (stat(dest.c_str(), &state) == OK && S_ISREG(state.st_mode))
			return true;
	}
	dest.clear();
	return false;
}

//  Method dispatch -
Response *Router::routeGET(struct stat &state)
{
	if (OK != access(_absolutePath.c_str(), R_OK))
		throw HttpError(HttpStatus::FORBIDDEN);

	int fileFd = open(_absolutePath.c_str(), O_RDONLY);
	if (fileFd < 0)
		throw HttpError(HttpStatus::INTERNAL_SERVER_ERROR);

	return new GETResponse(_request, state.st_size, fileFd, _clientFd,
						   _bufferManager);
}

/* Not yet implemented. */

Response *Router::routePOST() { throw HttpError(HttpStatus::NOT_IMPLEMENTED); }

Response *Router::routeDELETE()
{
	throw HttpError(HttpStatus::NOT_IMPLEMENTED);
}

Response *Router::routeCGI() { throw HttpError(HttpStatus::NOT_IMPLEMENTED); }
