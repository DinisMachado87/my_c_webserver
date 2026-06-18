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

	std::vector<Server *> &_servers;
	Server *_newServer;
	Location _newLocation;

	std::vector<StrView> &_defaultsVecBuff;

	uint _vecCursor;
	Token _token;
	Expect _expect;

	// Explicit disables
	ConfParser();
	ConfParser &operator=(const ConfParser &other);
	ConfParser(const ConfParser &other);

	// Methods
	bool isMethod();
	bool parseOverrides(Overrides &overrides);
	uchar parseServer();
	void nextServer();
	void parseServerLine();
	void parseLocation();
	void parseLocationParam();
	void parseMethod(Overrides &overrides);

	// Consolidation Methods
	void inheritUnsetParameters();
	void consolidatelocation(Location &loc, char *&dest);
	void consolidateBuffer();

	// Error handler
	std::runtime_error parsingErr(const char *expected) const;

public:
	// Constructors and destructors
	ConfParser(std::string &configStr, std::vector<Server *> &servers,
			   std::vector<StrView> &defaultsVecBuff);
	~ConfParser();

	// Methods
	void createServers();
};

#endif
