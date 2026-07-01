#ifndef ERRORRESPONSE_HPP
#define ERRORRESPONSE_HPP

#include "ASocket.hpp"
#include "HttpStatus.hpp"
#include "Response.hpp"

class ERRORResponse : public Response
{
private:
	ERRORResponse(const ERRORResponse &other);
	ERRORResponse &operator=(const ERRORResponse &other);
	ERRORResponse();

public:
	// Constructors and destructors
	ERRORResponse(const HttpStatus &status, const Request *request);
	~ERRORResponse();

	// Methods
	ASocket *execute();
	void createHTML();
};

#endif
