#ifndef HTTPTOKEN_HPP
#define HTTPTOKEN_HPP

#include "Token.hpp"

class HttpToken : public Token {
private:
	HttpToken();
	HttpToken(const HttpToken &other);

public:
	enum e_httpLine { SINGLE, DOUBLE };
	// Constructors and destructors
	HttpToken(std::string &parsingString);
	~HttpToken();
	// Operators overload
	HttpToken &operator=(const HttpToken &other);
	// Methods
	// uchar loadHttpNewLine();
	void loadNextHex(size_t *ret);
	StrView getBody(size_t bodySize);
	uchar handleNewline();
	static const uchar *configDelimiters();
};

#endif
