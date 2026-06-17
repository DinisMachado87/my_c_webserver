#ifndef CONFPARSER_HPP
#define CONFPARSER_HPP

#include "Expect.hpp"
#include "Overrides.hpp"
#include "Server.hpp"
#include "Token.hpp"
#include <stdexcept>
#include <string>
#include <vector>

class ConfParser {
private:
	enum e_state { NONE, SERVER, LOCATION, ENDOFILE };
	enum e_block { INIT_LOCATION, ENDLINE, ENDBLOCK, ENDFILE };

	Location _programDefaults;

	std::vector<Server *> &_servers;
	Server *_newServer;
	Location _newLocation;

	uint _vecCursor;
	Token _token;
	Expect _expect;

	// Explicit disables
	ConfParser();
	ConfParser &operator=(const ConfParser &other);
	ConfParser(const ConfParser &other);

	// Methods
	void inheritUnsetParameters();
	void consolidateBuffer();
	bool isMethod();
	void nextServer();
	uchar parseServer();
	void parseServerLine();
	void parseLocation();
	void parseLocationParam();
	bool parseOverrides(Overrides &overrides);
	void parseMethod(Overrides &overrides);

	// Consolidation Methods
	void consolidateBuffers();

	// Error handler
	std::runtime_error parsingErr(const char *expected) const;

public:
	// Constructors and destructors
	ConfParser(std::string &configStr, std::vector<Server *> &servers,
			   const Location &programDefaults);
	~ConfParser();

	// Methods
	void createServers();
};

#endif
