#include "ERRORResponse.hpp"
#include "ASocket.hpp"

// Public constructors and destructors
ERRORResponse::ERRORResponse(const HttpStatus &status, const Request *request) :
	Response(status, request)
{
}

ERRORResponse::~ERRORResponse() {}

ASocket *ERRORResponse::execute() { return NULL; }

void ERRORResponse::createHTML()
{
	const char *str = "HTTP/1.1 404 Not Found\r\n"
					  "Content-Type: text/html\r\n"
					  "Content-Length: 130\r\n"
					  "Connection: close\r\n \r\n"
					  "<html>"
					  "<head> <title> 404 Not Found</title></head>"
					  "<body> <h1> 404 Not Found</h1>"
					  "<p> The requested resource could not be found.</p>"
					  " </body>"
					  " </html> ";
	(void)str;
}
