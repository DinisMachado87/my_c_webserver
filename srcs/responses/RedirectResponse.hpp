#pragma once

#include "Request.hpp"
#include "Response.hpp"

class RedirectResponse : public Response
{
private:
	// Explicit Disables
	RedirectResponse();
	RedirectResponse &operator=(const RedirectResponse &other);
	RedirectResponse(const RedirectResponse &other);

public:
	// Constructors and destructors
	RedirectResponse(const Request *request);
	~RedirectResponse();

	// Operators overload

	// Getters and setters

	// Methods
};
