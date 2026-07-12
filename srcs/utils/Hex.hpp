#pragma once
#include <cstddef>

/* hex from/to size_t conversion.
 * No strtol: works on unterminated buffers, validates all chars, overflow-safe.
 * Holds both representations. */
class Hex
{
public:
	Hex();

	// Converts number to hex string. Populates both representations.
	void parseFromNum(size_t val);
	// Parses raw hex chars to number. Returns false on bad input.
	bool parseFromStr(const char *str, size_t len);

	size_t value() const;
	const char *str() const;
	size_t strLen() const;

private:
	size_t _value;
	char _str[16]; // max size_t in hex
	size_t _strLen;

	struct LookupTable {
		unsigned char digitToNum[256]; // 0xFF = invalid
		LookupTable();
	};
	static const LookupTable TABLE;
};
