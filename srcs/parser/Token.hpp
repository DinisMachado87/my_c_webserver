#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "StrView.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <sys/types.h>

class Token {
protected:
	const uchar *const _isDelimiter;

	StrView _parsingStr;
	StrView _strV;

	uchar _type;
	bool _needsMoreInput;

	size_t _strVBuffSize;

	friend class ConfParser;

private:
	// Explicit disables
	Token();
	Token(const Token &other);
	Token &operator=(const Token &other);

public:
	// Constructors and destructors
	Token(const uchar *table, const char *buffer, size_t size);
	~Token();

	enum e_Types {
		WORD,
		SPACE,
		NEWLINE,
		SEMICOLON,
		COMA,
		COMMENT,
		QUOTE,
		OPENBLOCK,
		CLOSEBLOCK,
		ENDOFILE,
		DIGIT,
		EXCAPE,
		OTHER
	};

	// Static Method for table generation
	static const uchar *configDelimiters();

	// Methods
	void loadParsingString(StrView parsingString);
	const char *findEndQuote(const char *str) const;
	void addToStrBuffSize();
	void resetNeedsMoreInputFlag();
	bool needsMoreInput();
	void loadNextChunk(const size_t size);
	bool loadNextHex(size_t *ret);
	void loadDigitsUntil(const char c);
	StrView getRemaining();
	size_t getSizeLeft() const;
	const char *getEnd() const;
	uchar loadNextStr(const char *errStr);
	uchar loadNextStr();
	uchar loadNextCore(const bool keepSpaces);
	uchar loadNext();
	uchar loadNextOfTypes(uchar *types, uint nTypes, const char *errStr);
	uchar loadNextOfType(uchar type, const char *errStr);
	std::runtime_error parsingErr(const char *expected) const;
	char compare(const char **strArr, uchar len);
	bool compare(StrView &strV) const;
	bool compare(const char *str) const;
	void extractQuote(const char *str);
	void printToken() const;
	void trackInUseToken(StrView *strV);
	// geters
	const char *getStart() const;
	uchar getType() const;
	StrView getStrV() const;
	std::string getString() const;
	size_t getStrBuffSize() const;
};

#endif
