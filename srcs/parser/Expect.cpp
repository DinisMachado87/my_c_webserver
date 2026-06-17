#include "Expect.hpp"
#include "Http.hpp"
#include "StrView.hpp"
#include "Token.hpp"
#include "webServ.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <map>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

using std::map;
using std::pair;
using std::string;
using std::vector;

// Public constructors and destructors
Expect::Expect(Token &token) :
	_token(token) {}

Expect::~Expect() {}

// Public Methods
uchar Expect::method() {
	static const uchar size = 4;

	for (int i = 1; i < size; i++)
		if (true == _token.compare(g_methods[i])) return (i);
	return DEFAULT;
}

bool Expect::onOff() {
	_token.loadNextOfType(Token::WORD, "\"on/off\"");
	if (_token.compare("on")) return (true);
	if (_token.compare("off")) return (false);
	throw _token.parsingErr("\"on/off\"");
}

uchar Expect::word(const char *str) {
	_token.loadNextOfType(Token::WORD, str);
	if (!_token.compare(str)) throw _token.parsingErr(str);
	return Token::WORD;
}

Span<StrView> Expect::wordVec(std::vector<StrView> &vecBuf, uint &vecCursor) {
	uint count = 0;
	while (1) {
		switch (_token.loadNext()) {
		case Token::WORD:
			vecBuf.push_back(_token.getStrV());
			_token.addToStrBuffSize();
			count++;
			break;
		case Token::SEMICOLON: {
			Span<StrView> ret(vecBuf, vecCursor, count);
			vecCursor += count;
			return ret;
		}
		default:
			throw _token.parsingErr("WORD");
		}
	}
}

void Expect::errorPage(map<uint, StrView> &errorMap) {
	uint code = nextInteger();
	StrView pathPtr;
	path(&pathPtr, UNTRACKED);
	// UNTRACKED - it will be consolidated manually because of reallocations

	errorMap.insert(std::make_pair(code, pathPtr));
	_token.addToStrBuffSize();
}

void Expect::path(StrView *dest, bool trackOn) {
	_token.loadNextOfType(Token::WORD, "/<PATH>");
	if (_token.getStrV().data()[0] == '/') {
		*dest = _token.getStrV();
		if (trackOn) _token.trackInUseToken(dest);
		return;
	}
	throw _token.parsingErr("/<PATH>");
}

void Expect::paths(StrView *paths, int n) {
	for (int i = 0; i < n; i++)
		path(&paths[i]);
}

uint Expect::findNextDivider(StrView &view) {
	uint newLen = view.size();
	if (1 >= newLen) return newLen;

	size_t nextDivider = view.find('/', 1);
	newLen = (nextDivider == string::npos) ? view.size()
										   : static_cast<uint>(nextDivider);
	return newLen;
}

size_t Expect::applySizeUnit(size_t value, char unit) {
	if (unit == '\0') return value;

	size_t multiplier;
	switch (tolower(unit)) {
	case 'k':
		multiplier = 1024;
		break;
	case 'm':
		multiplier = 1024 * 1024;
		break;
	case 'g':
		multiplier = 1024 * 1024 * 1024;
		break;
	default:
		throw _token.parsingErr("Invalid size unit (use k, m, or g)");
	}

	if (value > std::numeric_limits<size_t>::max() / multiplier)
		throw _token.parsingErr("Size value too large");
	return value * multiplier;
}

long Expect::number(const char **endPtr) {
	StrView token = _token.getStrV();
	const char *start = token.data();
	const char *tokenEnd = start + token.size();

	errno = 0;
	char *parseEnd;
	long result = strtol(start, &parseEnd, 10);

	if (errno == ERANGE) throw _token.parsingErr("Number out of range");
	if (parseEnd == start) throw _token.parsingErr("Expected number");
	if (parseEnd > tokenEnd) throw _token.parsingErr("Invalid number format");
	if (result < 0) throw _token.parsingErr("Negative number not allowed");

	*endPtr = parseEnd;
	return result;
}

int Expect::integer() {
	const char *end;
	long result = number(&end);

	StrView token = _token.getStrV();
	if (end != token.data() + token.size())
		throw _token.parsingErr("Unexpected characters after number");
	if (result > INT_MAX) throw _token.parsingErr("Number exceeds INT_MAX");

	return static_cast<int>(result);
}

int Expect::nextInteger() {
	_token.loadNextOfType(Token::WORD, "word");
	return integer();
}

size_t Expect::size() {
	_token.loadNextOfType(Token::WORD, "word");

	const char *end;
	long result = number(&end);
	size_t size = static_cast<size_t>(result);
	StrView token = _token.getStrV();
	const char *tokenEnd = token.data() + token.size();

	if (end != tokenEnd) {
		size = applySizeUnit(size, *end);
		if (++end != tokenEnd)
			throw _token.parsingErr("Invalid characters after size unit");
	}
	return size;
}

in_addr_t Expect::ip(string &ipStr) {
	const size_t nOctets = 4;

	if (ipStr == "*" || ipStr == "0.0.0.0") return INADDR_ANY;

	if (ipStr == "localhost") ipStr = "127.0.0.1";

	uchar octets[nOctets];
	size_t start = 0;

	for (size_t i = 0; i < nOctets; i++) {
		size_t dotPos = (i < 3) ? ipStr.find('.', start) : ipStr.length();

		if (dotPos == string::npos
			|| dotPos == start) // npos == not found/no position
			throw _token.parsingErr("Invalid IP address");

		string octetStr = ipStr.substr(start, dotPos - start);

		char *end;
		errno = 0;
		long octet = strtol(octetStr.c_str(), &end, 10);

		if (errno == ERANGE || *end != '\0' || octet < 0 || octet > 255)
			throw _token.parsingErr("Invalid IP address");

		octets[i] = static_cast<uchar>(octet);
		start = dotPos + 1;
	}

	uint32_t result = 0;
	result |= octets[0];
	result |= octets[1] << 8;
	result |= octets[2] << 16;
	result |= octets[3] << 24;
	return static_cast<in_addr_t>(result);
}

uint16_t Expect::port(const string &portStr) {
	if (portStr.empty()) throw _token.parsingErr("Invalid port number");

	char *end;
	errno = 0;
	long port = strtol(portStr.c_str(), &end, 10);

	if (errno == ERANGE || *end != '\0' || port < 1 || port > 65535)
		throw _token.parsingErr("Port must be between 1 and 65535");

	return static_cast<uint16_t>(port);
}
