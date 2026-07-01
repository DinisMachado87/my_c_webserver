#include "HttpToken.hpp"
#include "StrView.hpp"
#include "Token.hpp"
#include "webServ.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdlib>

using std::string;

// Public constructors and destructors
HttpToken::HttpToken(StrView parsingString) :
	Token(httpDelimiters(), parsingString.data(), parsingString.size()) {}

HttpToken::~HttpToken() {}

// Public Methods
const uchar *HttpToken::httpDelimiters() {
	static uchar isDelimiter[256] = {0};
	isDelimiter[' '] = Token::SPACE;
	isDelimiter['\t'] = Token::SPACE;
	isDelimiter['\r'] = Token::NEWLINE;
	isDelimiter['\n'] = Token::NEWLINE;
	isDelimiter['\0'] = Token::ENDOFILE;
	return isDelimiter;
}

uchar HttpToken::handleNewline() {
	uint size = _parsingStr.size();
	uint equalLen = (size >= 4) ? 4 : size;

	uint i = 0;
	while (i < equalLen && _parsingStr.data()[i] == "\r\n\r\n"[i])
		i++;
	equalLen = i;

	if (equalLen == 4) {
		_strV = StrView(_parsingStr.data(), equalLen);
		_parsingStr.removePrefix(equalLen);
		_type = NEWLINE;
		return DOUBLE;
	}
	// case "\r\n\r\0" | "\r\n\0" | "\r\0"
	if (equalLen == size) {
		_needsMoreInput = true;
		return OTHER;
	}
	// case "\r\nFollowedByOtherCharacters"
	if (equalLen >= 2) {
		_strV = StrView(_parsingStr.data(), equalLen);
		_parsingStr.removePrefix(equalLen);
		_type = NEWLINE;
		return SINGLE;
	}
	// not a new line
	throw parsingErr("End of line or request");
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
