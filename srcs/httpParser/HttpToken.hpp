#ifndef HTTPTOKEN_HPP
#define HTTPTOKEN_HPP

#include "StrView.hpp"
#include "Token.hpp"

/* Token subclass with the HTTP delimiter table — treats \r\n as
 * newline and has no block/semicolon/comment/quote rules.
 * Adds CRLF handling and chunked-encoding helpers. */
class HttpToken : public Token
{
public:
	enum e_httpLine { SINGLE, DOUBLE };

	// Takes StrView by value — caller keeps an unchanged copy.
	HttpToken(StrView parsingString);
	~HttpToken();

	/* Operators overload*/
	HttpToken &operator=(const HttpToken &other);

	bool loadNextWord();
	void loadNextHex(size_t *ret);
	StrView getBody(size_t bodySize);
	StrView leftover() const;

	/* CRLF detection — returns SINGLE (\r\n), DOUBLE (\r\n\r\n),
	 * or sets _needsMoreInput if the sequence is truncated. */
	// uchar handleNewline();
	bool isNewLine();
	bool consumeNewLine();

	// Builds the HTTP delimiter table (spaces + newlines only).
	static const uchar *httpDelimiters();

private:
	/* Explicit disables */
	HttpToken();
	HttpToken(const HttpToken &other);
};

#endif
