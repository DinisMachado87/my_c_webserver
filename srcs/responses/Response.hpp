#pragma once

#include "HttpStatus.hpp"
#include "Request.hpp"
#include "webServ.hpp"
class Location;
class ASocket;

/* Base response — owns the Request that triggered it (deletes on destruction).
 * Subclasses override execute/recvBody/send for each response type. */
class Response
{
private:
	/* Explicit disables*/
	Response();
	Response(const Response &other);

protected:
	const HttpStatus &_status;
	const Request *_request;

	uchar _flags;
	enum e_FLAGS_BIT_N { RECV, SEND };

public:
	Response(const HttpStatus &status, const Request *request);
	~Response();

	/* Operators overload */
	Response &operator=(const Response &other);

	/* Virtual interface — defaults are no-ops */
	virtual ASocket *execute();
	virtual void recvBody();
	virtual int send();
};
