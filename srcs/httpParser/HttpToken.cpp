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
	Token(httpDelimiters(), parsingString.data(), parsingString.size())
{
}

HttpToken::~HttpToken() {}

// Public Methods
const uchar *HttpToken::httpDelimiters()
{
	static uchar isDelimiter[256] = {0};
	isDelimiter[' '] = Token::SPACE;
	isDelimiter['\t'] = Token::SPACE;
	isDelimiter['\r'] = Token::NEWLINE;
	isDelimiter['\n'] = Token::NEWLINE;
	return isDelimiter;
}

bool HttpToken::isNewLine()
{
	const char *str = _parsingStr.data();
	uint size = _parsingStr.size();
	if (!size || str[0] != '\r')
		return false;
	if (size < 2) {
		_needsMoreInput = true;
		return false;
	}
	return str[1] == '\n';
}

bool HttpToken::consumeNewLine()
{
	if (isNewLine()) {
		_parsingStr.removePrefix(2);
		return DONE;
	}
	if (_needsMoreInput)
		return ONGOING;
	throw parsingErr("\\r\\n");
}

StrView HttpToken::getBody(size_t bodySize)
{
	if (getSizeLeft() < bodySize)
		_needsMoreInput = true;
	return getRemaining();
}

bool HttpToken::loadNextWord()
{
	if (loadNext() == ENDOFILE)
		return ONGOING;
	if (_type != WORD)
		throw parsingErr("Expected word");
	return DONE;
}

void HttpToken::loadNextHex(size_t *ret)
{
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
	if (2 > sizeLeft)
		_needsMoreInput = true;
	if (_needsMoreInput)
		return;
	if (end == start)
		throw parsingErr("Expected number");

	_strV.setSize(len);

	// if (!loadHttpNewLine() || NEWLINE != getType())
	// 	throw parsingErr("Newline");

	*ret = static_cast<size_t>(num);
	return;
}

StrView HttpToken::leftover() const { return _parsingStr; }
