#ifndef RESPONSE_HPP
#define RESPONSE_HPP

class Location;
class Request;

class Response {
private:
	// Explicit disables
	Response();
	Response(const Response &other);

protected:
	const Request *_request;
	const Location &_location;

public:
	// Constructors and destructors
	Response(const Request *request, const Location &location);
	~Response();

	// Operators overload
	Response &operator=(const Response &other);

	// Getters and setters

	// Methods
	int send();
};

#endif
