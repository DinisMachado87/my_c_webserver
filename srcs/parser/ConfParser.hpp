#ifndef CONFPARSER_HPP
#define CONFPARSER_HPP

#include "Expect.hpp"
#include "Overrides.hpp"
#include "Server.hpp"
#include "Token.hpp"
#include <stdexcept>
#include <string>
#include <vector>

/* Recursive-descent parser for the config file.
 * - Builds Server objects,
 * - then consolidates all StrViews into each Server's owned string buffer
 *   	to make memory contibuous and potenciate cache
 *   	since config happens once in the beginning of the program
 *   	and is accessed all the time during runtime.
 * 		Original config read can be freed. */
class ConfParser
{
public:
	// Constructors and destructors
	ConfParser(std::string &configStr, std::vector<Server *> &servers,
			   std::vector<StrView> &defaultsVecBuff);
	~ConfParser();

	// Top-level entry point. Throws on any syntax error.
	void createServers();

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

	/* Explicit disables */
	ConfParser();
	ConfParser &operator=(const ConfParser &other);
	ConfParser(const ConfParser &other);

	static const uchar *configDelimiters();

	/* Recursive-descent entry points
	 * — mirror the config grammar:
	 * 		server { serverLine | location { locationLine } } */
	void nextServer();
	void parseServerLine();
	void parseLocation();
	void parseLocationline();
	bool parseOverrides(Overrides &overrides);
	void parseListen();
	void parseMethod(Overrides &overrides);
	uchar parseServer();
	bool isMethod();

	/* Consolidation Methods
	 * - Copies all StrViews into the Server's owned buffer
	 * - Copies unassigned values following inheritance chain: program -> server
	 * -> location. This concentrates comparisons once while parsing rather than
	 * with every searc during runtime */
	void consolidateAndStoreNewServer();
	void consolidatelocation(Location &loc, char *&dest);
	void consolidateBuffer();

	std::runtime_error parsingErr(const char *expected) const;
};

#endif
