#include "HttpToken.hpp"
#include "Token.hpp"

using std::string;

// Public constructors and destructors
HttpToken::HttpToken(std::string &parsingString) :
	Token(configDelimiters(), parsingString) {}

HttpToken::~HttpToken() {}

// Public Methods
const uchar *HttpToken::configDelimiters() {
	static uchar isDelimiter[256] = {0};
	isDelimiter[' '] = Token::SPACE;
	isDelimiter['\t'] = Token::SPACE;
	isDelimiter['\r'] = Token::NEWLINE;
	isDelimiter['\n'] = Token::NEWLINE;
	isDelimiter['\0'] = Token::ENDOFILE;
	return isDelimiter;
}

uchar HttpToken::handleNewline() {
	loadNextOfType(NEWLINE, "NEWLINE");

	if (compare("\r\n\r\n"))
		return SINGLE;
	else if (compare("\r\n"))
		return DOUBLE;
	throw parsingErr("NEWLINE");
}
