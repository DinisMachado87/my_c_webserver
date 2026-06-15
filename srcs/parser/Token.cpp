#include "Token.hpp"
#include "Logger.hpp"
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
#include <vector>

using std::map;
using std::string;
using std::stringstream;
using std::strtol;
using std::vector;

// Public constructors and destructors
Token::Token(const uchar *table, char *parsingString, size_t size) :
	_isDelimiter(table),
	_parsingStr(StrView(parsingString, size)),
	_pendingQuote(false),
	_needsMoreInput(false),
	_vecBuffConsolidationIndex(0) {}

Token::~Token() {}

// Error Handling
std::runtime_error Token::parsingErr(const char *expected) const {
	std::ostringstream oss;
	oss << "Error Parsing config: "
		<< "Expected \"" << expected << "\" "
		<< "got \"" << getString() << "\" "
		<< "in line " << getLineN() << "\"";

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

void Token::extractQuote(const char *str) {
	str++;

	while (1) {
		_type = _isDelimiter[(uchar)(*str)];
		switch (_type) {
		case NEWLINE:
		case ENDOFILE:
			throw std::runtime_error("Error tokenizer: unclosed quote");
		case EXCAPE:
			str++;
			if (*str)
				str++;
			continue;
		case QUOTE:
			_strV.setSize(str - _strV.data());
			_pendingQuote = true;
			return;
		default:
			str++;
		}
	}
}

uchar Token::loadNext() { return loadNextCore(false); }
uchar Token::loadNextStr() { return loadNextCore(true); }

uchar Token::loadNextCore(const bool keepSpaces) {
	_strV.removePrefix(_strV.size() + _pendingQuote);
	_pendingQuote = false;
	const char *str = _strV.data();
	_strV.setSize(0);

	while (1) {
		_type = _isDelimiter[(uchar)(*str)];
		switch (_type) {
		case ENDOFILE:
			_needsMoreInput = true;
			return ENDOFILE;
		case NEWLINE: // extract new line expresion
			_strV.setStart(str);
			while (NEWLINE == _isDelimiter[(uchar)(*str)])
				str++;
			if (ENDOFILE == _isDelimiter[(uchar)(*str)])
				_needsMoreInput = true;
			_strV.setSize(str - _strV.data());
			return _type;
		case SPACE: // Skip cursor
			str++;
			break;
		case COMMENT: // Skip comment
			while (*str && *str != '\n')
				str++;
			break;

		case WORD: // Extract Token
			_strV.setStart(str);
			if (keepSpaces)
				while (WORD == _isDelimiter[(uchar)(*str)]
					   || SPACE == _isDelimiter[(uchar)(*str)])
					str++;
			else
				while (WORD == _isDelimiter[(uchar)(*str)])
					str++;

			if (ENDOFILE == _isDelimiter[(uchar)(*str)]) {
				_needsMoreInput = true;
				_type = WORD;
			} else
				_strV.setSize(str - _strV.data());

			return _type;

		case QUOTE:
			str++;
			_strV.setStart(str);
			extractQuote(str);
			return _type = WORD;

		default:
			_strV.setStart(str);
			_strV.setSize(1);
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
	const uchar len = _strV.size();
	if (OK == strncmp(_strV.data(), str, len) && str[len] == '\0')
		return true;
	return false;
};

bool Token::compare(StrView &strV) const { return compare(strV.data()); };

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
size_t Token::getSizeLeft() const { return (_strBuffSize - _strV.size()); }

StrView Token::getRemaining() {
	_strV.removePrefix(_strV.size());
	_strV.setSize(getSizeLeft());
	return _strV;
}

void Token::loadNextChunk(const size_t size) {
	_strV.removePrefix(_strV.size());
	_strV.setSize(size);
}

string Token::getString() const { return (string(_strV.data(), _strV.size())); }

void Token::trackInUseToken(StrView *strV) {
	_tokensInUse.push_back(strV);
	_strBuffSize += strV->size() + 1;
}

void Token::printBuffer() { _strV.printBuffer(); }

void Token::printBuffers(stringstream &stream) {
	stream << "_tokensInUse: ";
	for (size_t i = 0; i < _tokensInUse.size(); i++)
		stream << _tokensInUse[i]->getStr() << "\n";
}

void Token::consolidateStrVMap(map<uint, StrView> &strVMap, char *newStrBuf) {
	LOG(Logger::LOG, "Consolidating Map Buffer: ");
	map<uint, StrView>::iterator cur = strVMap.begin();
	for (; cur != strVMap.end(); ++cur)
		cur->second.move(newStrBuf);
}

void Token::consolidateStrVSpans(vector<StrView> &vecBuf, char *newStrBuf) {
	LOG(Logger::LOG, "Consolidating Span Buffer: ");

	size_t i = _vecBuffConsolidationIndex;
	LOGNUM(Logger::LOG, "_vecBuffConsolidation index: ", i);
	for (; i < vecBuf.size(); i++)
		vecBuf[i].move(newStrBuf);
	_vecBuffConsolidationIndex = vecBuf.size();
}

void Token::consolidateBuffer(char *newBuf) {
	LOG(Logger::LOG, "Consolidating StrBuffer: ");

	for (uint i = 0; i < _tokensInUse.size(); i++)
		_tokensInUse[i]->move(newBuf);
	_tokensInUse.clear();
}

void Token::consolidateBuffers(vector<StrView> &vecBuf, char *dest,
							   size_t destSize) {
	consolidateBuffer(dest);
	consolidateStrVSpans(vecBuf, dest);
	_strBuffSize = 0;
}

bool Token::needsMoreInput() { return _needsMoreInput; }
void Token::resetNeedsMoreInputFlag() { _needsMoreInput = false; }
void Token::resetSpanConsolidationIndex() { _vecBuffConsolidationIndex = 0; }
