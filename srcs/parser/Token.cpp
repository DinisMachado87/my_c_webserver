#include "Token.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>

using std::string;
using std::stringstream;

// Public constructors and destructors
Token::Token(const uchar *table, const char *parsingString, size_t size) :
	_isDelimiter(table),
	_parsingStr(StrView(parsingString, size)),
	_strV(StrView(parsingString, size)),
	_needsMoreInput(false),
	_strVBuffSize(0),
	_vecBuffConsolidationIndex(0) {}

Token::~Token() {}

// Error Handling
std::runtime_error Token::parsingErr(const char *expected) const {
	std::ostringstream oss;
	oss << "Error Parsing config: "
		<< "Expected \"" << expected << "\" "
		<< "got \"" << getString() << "\" ";

	return std::runtime_error(oss.str());
}

// Public Methods
const uchar *Token::configDelimiters() {
	static uchar isDelimiter[256] = {0};
	isDelimiter[' '] = SPACE;
	isDelimiter['\t'] = SPACE;
	isDelimiter['\n'] = SPACE;
	isDelimiter['#'] = COMMENT;
	isDelimiter['"'] = QUOTE;
	isDelimiter['{'] = OPENBLOCK;
	isDelimiter['}'] = CLOSEBLOCK;
	isDelimiter[';'] = SEMICOLON;
	isDelimiter['\\'] = EXCAPE;
	isDelimiter['\0'] = ENDOFILE;
	return isDelimiter;
}

const char *Token::findEndQuote(const char *str) const {
	while (1) {
		switch (_isDelimiter[(uchar)(*str)]) {
		case ENDOFILE:
			throw std::runtime_error("Error tokenizer: unclosed quote");
		case EXCAPE:
			str++;
			if (*str)
				str++;
			continue;
		case QUOTE:
			return str;
		default:
			str++;
		}
	}
}

uchar Token::loadNext() { return loadNextCore(false); }
uchar Token::loadNextStr() { return loadNextCore(true); }
uchar Token::loadNextCore(const bool keepSpaces) {
	const char *cur = _parsingStr.data();

	while (1) {
		_type = _isDelimiter[(uchar)(*cur)];
		switch (_type) {
		case ENDOFILE:
			_needsMoreInput = true;
			_parsingStr.setStart(cur); // keep cursor consistent
			_parsingStr.setSize(0);
			return ENDOFILE;

		case SPACE:
			cur++;
			break;

		case COMMENT:
			while (*cur && *cur != '\n')
				cur++;
			break;

		case WORD: {
			const char *tokStart = cur;
			if (keepSpaces)
				while (WORD == _isDelimiter[(uchar)(*cur)]
					   || SPACE == _isDelimiter[(uchar)(*cur)])
					cur++;
			else
				while (WORD == _isDelimiter[(uchar)(*cur)])
					cur++;
			_strV = StrView(tokStart, cur - tokStart);
			_parsingStr.removePrefix(cur - _parsingStr.data());
			return _type = WORD;
		}

		case QUOTE: {
			cur++;
			const char *tokStart = cur;
			const char *tokEnd = findEndQuote(cur);
			const char *closingQuote = tokEnd + 1;
			_strV = StrView(tokStart, tokEnd - tokStart);
			_parsingStr.removePrefix(closingQuote - _parsingStr.data());
			return _type = WORD;
		}

		default:
			_strV = StrView(cur, 1);
			_parsingStr.removePrefix((cur + 1) - _parsingStr.data());
			return _type;
		}
	}
}

uchar Token::loadNextOfType(uchar type, const char *errStr) {
	loadNext();
	if (type != _type)
		throw parsingErr(errStr);
	return _type;
}

uchar Token::loadNextStr(const char *errStr) {
	loadNextStr();
	if (WORD != _type)
		throw parsingErr(errStr);
	return _type;
}

uchar Token::loadNextOfTypes(uchar *types, uint nTypes, const char *errStr) {
	loadNext();

	while (nTypes--) {
		if (*types == _type)
			return *types;
		types++;
	}
	throw parsingErr(errStr);
}

bool Token::compare(const char *str) const {
	const size_t charLen = strlen(str);
	const size_t strVLen = _strV.size();
	if (charLen != strVLen)
		return false;
	if (OK == strncmp(_strV.data(), str, strVLen))
		return true;
	return false;
};

bool Token::compare(StrView &strV) const { return _strV.compare(strV); };

char Token::compare(const char **strArr, uchar len) {
	for (uchar i = 0; i < len; i++)
		if (OK == compare(strArr[i]))
			return i;
	return -1;
}

uchar Token::getType() const { return _type; }
StrView Token::getStrV() const { return _strV; }
const char *Token::getStart() const { return _strV.data(); }
const char *Token::getEnd() const { return _strV.end(); }
size_t Token::getSizeLeft() const { return (_strVBuffSize - _strV.size()); }

StrView Token::getRemaining() {
	_strV.removePrefix(_strV.size());
	_strV.setSize(getSizeLeft());
	return _strV;
}

void Token::loadNextChunk(const size_t size) {
	_strV.removePrefix(_strV.size());
	_strV.setSize(size);
}

string Token::getString() const { return _strV.getStr(); }
size_t Token::getStrBuffSize() const { return _strVBuffSize; }
void Token::addToStrBuffSize() { _strVBuffSize += _strV.size(); }
bool Token::needsMoreInput() { return _needsMoreInput; }
void Token::resetNeedsMoreInputFlag() { _needsMoreInput = false; }
