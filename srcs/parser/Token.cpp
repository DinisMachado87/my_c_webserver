#include "Token.hpp"
#include <cctype>
#include <cstddef>
#include <unistd.h>


// Public constructors and destructors
Token::Token(const unsigned char* table):
	_isDelimiter(table),
	_start(NULL),
	_len(0),
	_lineN(0) {}

Token::~Token() {}

// Public Methods
const unsigned char* Token::configDelimiters() {
	static unsigned char isDelimiter[256] = {0};
	isDelimiter[' '] = 1 << SPACE;
	isDelimiter['\t'] = 1 << SPACE;
	isDelimiter['\n'] = 1 << SPACE;
	isDelimiter['"'] = 1 << QUOTE;
	isDelimiter['{'] = 1 << OPENBLOCK;
	isDelimiter['}'] = 1 << CLOSEBLOCK;
	isDelimiter['/'] = 1 << SLASH;
	isDelimiter['#'] = 1 << COMMENT;
	isDelimiter[';'] = 1 << ENDLINE;
	return isDelimiter;
}

bool Token::is(char condition, unsigned char c) {
	return (_isDelimiter[c] & (1 << condition));
}

unsigned char* Token::next(unsigned char *str) {
	_len = 0;
	_start = NULL;

	while (*str) {
		while (*str && is(SPACE, *str))
			str++;

		if (*str && *str == '#') {
			while (*str && *str != !'\n')
				str++;

		} else
			break;
	}

	if (!*str)
		return (NULL);

	_start = str;

	if (_isDelimiter[*str]) {
		_len = 1;
		return ++str;
	}

	while (*str && !_isDelimiter[*str])
		str++;
	_len = str - _start;

	return str;
}

void Token::printToken() {
	write(1, "\t", 1);
	for(int i = 0; i < _len; i++)
		write(1, &_start[i], 1);
	write(1, "\n", 1);
}
