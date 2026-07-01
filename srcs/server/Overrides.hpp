#ifndef OVERRIDES_HPP
#define OVERRIDES_HPP

#include "Span.hpp"
#include "StrView.hpp"
#include <cstddef>
#include <map>
#include <ostream>
#include <vector>

/* Bitmask flags — tracks which fields were explicitly set in config
 * so inheritUnsetParams knows what to fill from the parent tier. */
enum e_Field {
	F_ROOT,
	F_INDEX,
	F_AUTOINDEX,
	F_CLIENT_BODY,
	F_ERROR,
	F_METHODS,
	F_NOT_OVERRIDES_FIELD,
};

/* Inheritable config fields shared by server and location blocks.
 * Cascade: program defaults -> server -> location.
 * Only fields not explicitly set (_set bitmask) inherit from parent. */
class Overrides
{
public:
	Overrides(std::vector<StrView> &vecBuf);

	// Copies each unset field from parent. Error pages are unioned — child wins
	// per-code.
	void inheritUnsetParams(const Overrides &parent);

	/* Getters */
	bool isAutoindexed() const;
	const Span<StrView> &getIndex() const;
	const StrView &getRoot() const;
	const char *findErrorFile(uint errorCode) const;
	size_t getErrorMapSize() const;
	size_t getClientMaxBody() const;
	uchar isAllowedMethod(uchar methodToCheck) const;
	uchar getAllowedMethods() const;

	/* Print */
	void printMethods(std::ostream &stream) const;
	void printOverrides(const char *label, std::ostream &stream) const;
	void printMap(const char *label, std::ostream &stream) const;

private:
	static const char *_methodStrs[4];

	std::map<uint, StrView> _error; // error code -> file path
	Span<StrView> _index;
	StrView _root;
	size_t _clientMaxBody;
	bool _autoindex;
	uchar _allowedMethods; // bitfield — (1 << GET) | (1 << POST) | ...
	uchar _set;			   // bitmask of e_Field — tracks explicit assignments

	/* Builds temporary hardcoded program defaults for parsing.
	 * Only used by ConfParser. */
	Overrides(std::vector<StrView> &vecBuf, uchar ProgramDefaultsAllSet);

	friend class ConfParser;
	friend class ConfParserTest;
};

#endif
