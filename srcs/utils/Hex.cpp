#include "Hex.hpp"
#include <cstddef>
#include <cstring>

static const unsigned char INVALID = 0xFF;

Hex::LookupTable::LookupTable()
{
	memset(digitToNum, INVALID, sizeof(digitToNum));
	for (int i = 0; i < 10; i++)
		digitToNum['0' + i] = i;
	for (int i = 0; i < 6; i++) {
		digitToNum['a' + i] = 10 + i;
		digitToNum['A' + i] = 10 + i;
	}
}

const Hex::LookupTable Hex::TABLE;

/* Constructors */

Hex::Hex() :
	_value(0),
	_strLen(0)
{
	_str[0] = '\0';
}

/* Public Methods */

void Hex::parseFromNum(const size_t value)
{
	if (value == 0) {
		_str[0] = '0';
		_strLen = 1;
		_value = 0;
		return;
	}

	size_t cur = 0;
	for (size_t tmp = value; tmp; tmp /= 16)
		cur++;
	_strLen = cur;
	_value = value;

	const char DIGITS[17] = "0123456789abcdef";
	size_t tmp = value;
	while (cur--) {
		_str[cur] = DIGITS[tmp % 16];
		tmp /= 16;
	}
}

bool Hex::parseFromStr(const char *str, size_t len)
{
	if (len == 0 || len > 16)
		return false;

	static const size_t allBitsOn = static_cast<size_t>(-1);
	static const size_t maxBeforeMultiply = allBitsOn / 16;

	size_t total = 0;
	for (size_t i = 0; i < len; i++) {
		unsigned char digit
			= TABLE.digitToNum[static_cast<unsigned char>(str[i])];
		if (digit == INVALID)
			return false;
		if (total > maxBeforeMultiply)
			return false;
		total = total * 16 + digit;
	}
	_value = total;
	memcpy(_str, str, len);
	_strLen = len;
	return true;
}

size_t Hex::value() const { return _value; }
const char *Hex::str() const { return _str; }
size_t Hex::strLen() const { return _strLen; }
