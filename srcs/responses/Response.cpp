#include "Response.hpp"
#include "Request.hpp"

Response::Response(const HttpStatus &status, const Request *request) :
	_status(status),
	_request(request),
	_flags(0)
{
}

Response::~Response() { delete _request; }

ASocket *Response::execute() { return NULL; };
void Response::recvBody() {};
int Response::send() { return 0; };
