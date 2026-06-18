#include "HttpToken.hpp"
#include "StrView.hpp"
#include "Token.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdlib>

using std::string;

// Public constructors and destructors
HttpToken::HttpToken(std::string &parsingString) :
	Token(configDelimiters(), parsingString.c_str(), parsingString.size()) {}

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

// uchar HttpToken::loadHttpNewLine() {
// 	_strV.updateOffset(_strV.getLen());
// 	_strV.setSize(0);
//
// 	size_t sizeLeft = getSizeLeft();
// 	if (sizeLeft < 2)
// 		return OTHER;
//
// 	if (_strV.compare("\r\n")) {
// 		_strV.setSize(2);
// 		return NEWLINE;
// 	} else if (sizeLeft < 2)
// 		return OTHER;
// 	else if (_strV.compare("\0\r\n")) {
// 		_strV.setSize(3);
// 		return ENDOFILE;
// 	}
// 	throw parsingErr("End of line or request");
// }

uchar HttpToken::handleNewline() {
	loadNextOfType(NEWLINE, "NEWLINE");

	if (compare("\r\n\r\n"))
		return DOUBLE;
	else if (compare("\r\n"))
		return SINGLE;
	if (_needsMoreInput)
		return ENDOFILE;
	throw parsingErr("NEWLINE");
}

StrView HttpToken::getBody(size_t bodySize) {
	if (getSizeLeft() < bodySize)
		_needsMoreInput = true;
	return getRemaining();
}

void HttpToken::loadNextHex(size_t *ret) {
	static const int hexLen = 16;
	_strV.removePrefix(_strV.size());
	const char *start = _strV.data();

	errno = 0;
	char *end;
	long num = strtol(start, &end, hexLen);

	if (errno == ERANGE)
		throw parsingErr("Number out of range");
	if (num < 0)
		throw parsingErr("Negative chunk len received");

	// Check if there is space to be "\r\n" or "\0\r\n" terminated
	size_t len = end - start;
	size_t sizeLeft = getSizeLeft() - len;
	if ((*end == '\0' && 3 > sizeLeft) || 2 > sizeLeft)
		_needsMoreInput = true;
	return;

	if (end == start)
		throw parsingErr("Expected number");

	_strV.setSize(len);

	// if (!loadHttpNewLine() || NEWLINE != getType())
	// 	throw parsingErr("Newline");

	*ret = static_cast<size_t>(num);
	return;
}
