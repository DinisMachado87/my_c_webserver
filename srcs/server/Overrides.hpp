#ifndef OVERRIDES_HPP
#define OVERRIDES_HPP

#include "Span.hpp"
#include "StrView.hpp"
#include <cstddef>
#include <map>
#include <ostream>
#include <vector>

enum e_Field {
	F_ROOT,
	F_INDEX,
	F_AUTOINDEX,
	F_CLIENT_BODY,
	F_ERROR,
	F_METHODS,
	F_NOT_OVERRIDES_FIELD,
};

class Overrides {
private:
	static const char *_methodStrs[4];

	std::map<uint, StrView> _error;
	Span<StrView> _index;
	StrView _root;
	size_t _clientMaxBody;
	bool _autoindex;
	uchar _allowedMethods;
	uchar _set;

	friend class ConfParser;
	friend class ConfParserTest;

public:
	Overrides(std::vector<StrView> &vecBuf);

	// Methods
	void mergeFrom(const Overrides &parent);
	// Getters
	bool isAutoindexed() const;
	const Span<StrView> &getIndex() const;
	const char *getRoot() const;
	const char *findErrorFile(uint errorCode) const;
	size_t getErrorMapSize() const;
	size_t getClientMaxBody() const;
	uchar isAllowedMethod(uchar methodToCheck) const;
	uchar getAllowedMethods() const;

	// Print
	void printMethods(std::ostream &stream) const;
	void printOverrides(const char *label, std::ostream &stream) const;
	void printMap(const char *label, std::ostream &stream) const;
};

#endif
