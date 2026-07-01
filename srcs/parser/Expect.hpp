#ifndef EXPECT_HPP
#define EXPECT_HPP

#include "Span.hpp"
#include "Token.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <map>
#include <netinet/in.h>
#include <string>
#include <vector>

/* Validation layer on top of Token. Each method advances the tokenizer,
 * checks the value against expected format, and returns the typed result
 * or throws on mismatch. */
class Expect
{
public:
	// Constructors and destructors
	Expect(Token &token);
	~Expect();

	/* Token-type expectations
	 * — all throw on mismatch */
	unsigned char word(const char *str1);
	uchar method(); // returns method index, or DEFAULT if unknown
	bool onOff();	// expects "on"/"off"
	int integer();
	int nextInteger(); // loads next token then calls integer()
	size_t size();	   // number with optional k/m/g suffix
	uint16_t port(const std::string &portStr);
	in_addr_t ip(std::string &ipStr); // "*" and "localhost" accepted

	/* Complex expectations */
	void path(StrView *dest); // must start with '/'
	void paths(StrView *paths, int n);
	void errorPage(std::map<uint, StrView> &errorMap); // code + path pair
	// Collects words until ';'. Returns a Span into vecBuf.
	Span<StrView> wordVec(std::vector<StrView> &vecBuf, uint &vecCursor);

	/* Path utilities */
	uint advanceToNextSegment(StrView &view, uint currentLen);
	uint findNextDivider(StrView &view);
	void consolidatePath(std::vector<StrView> &segments, size_t &writeIdx,
						 bool &hasChanges);
	void printPathSegs(std::vector<StrView> &segments, uint i, uint writeIdx,
					   uint deletedSegs);

private:
	Token &_token;

	/* Explicit disables */
	Expect(const Expect &other);
	Expect &operator=(const Expect &other);

	long number(const char **endPtr); // raw strtol with range/sign checks
	size_t applySizeUnit(size_t value, char unit); // k/m/g multiplier
};

#endif
