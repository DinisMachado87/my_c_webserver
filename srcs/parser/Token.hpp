#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "StrView.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <sys/types.h>

/* Recursive-descent parser for the config file.
 * Builds Server objects, then consolidates all StrViews into
 * each Server's owned buffer — contiguous memory for cache locality
 * since config is parsed once at startup and read throughout runtime.
 * Original config source can be freed after consolidation. */
class Token
{
public:
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

	/* Core tokenization */
	uchar loadNext();	 // skips whitespace and comments
	uchar loadNextStr(); // preserves spaces within token
	uchar loadNextCore(const bool keepSpaces);
	uchar loadNextOfType(uchar type, const char *errStr); // throws on mismatch
	uchar loadNextOfTypes(uchar *types, uint nTypes, const char *errStr);
	uchar loadNextStr(const char *errStr);

	/* Incremental input — for HTTP parsing where data arrives in chunks */
	void loadParsingString(StrView parsingString);
	void loadNextChunk(const size_t size);
	StrView getRemaining();
	size_t getSizeLeft() const;
	bool needsMoreInput();
	void resetNeedsMoreInputFlag();

	/* Comparison */
	bool compare(const char *str) const;
	bool compare(StrView &strV) const;
	char compare(const char **strArr, uchar len); // returns index or -1

	/* Accessors */
	const char *getStart() const;
	const char *getEnd() const;
	uchar getType() const;
	StrView getStrV() const;
	std::string getString() const;
	size_t getStrBuffSize() const;

	void addToStrBuffSize();
	const char *findEndQuote(const char *str) const; // throws on unclosed quote
	std::runtime_error parsingErr(const char *expected) const;
	bool loadNextHex(size_t *ret);
	void loadDigitsUntil(const char c);
	void extractQuote(const char *str);
	void printToken() const;
	void trackInUseToken(StrView *strV);

protected:
	const uchar *const _isDelimiter; // 256-byte classification table

	StrView _parsingStr; // unconsumed input
	StrView _strV;		 // current token

	uchar _type;
	bool _needsMoreInput; // set when ENDOFILE reached mid-parse

	size_t _strVBuffSize; // tracks total bytes to consolidate

	friend class ConfParser;

private:
	/* Explicit disables */
	Token();
	Token(const Token &other);
	Token &operator=(const Token &other);
};

#endif
