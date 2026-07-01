#include "RedirectResponse.hpp"
#include "HttpStatus.hpp"
#include "Request.hpp"

// Public constructors and destructors
RedirectResponse::RedirectResponse(const Request *request) :
	Response(HttpStatus::MOVED_PERMANENTLY, request)
{
}

RedirectResponse::~RedirectResponse() {}

// Public Methods
