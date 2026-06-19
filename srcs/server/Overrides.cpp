#include "Overrides.hpp"
#include "Span.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <bitset>
#include <cstddef>
#include <map>
#include <ostream>
#include <sys/types.h>

using std::bitset;
using std::map;
using std::ostream;
using std::size_t;
using std::vector;

const char *Overrides::_methodStrs[4] = {"DEFAULT", "GET", "POST", "DELETE"};

// Constructor
Overrides::Overrides(std::vector<StrView> &vecBuf) :
	_index(vecBuf),
	_clientMaxBody(0),
	_autoindex(false),
	_allowedMethods(DEFAULT),
	_set(0) {}

/* Protected constructor for temp Programdefaults used in ConfParser
// inheritUnsetValues() */
Overrides::Overrides(vector<StrView> &vecBuf, uchar ProgramDefaultsAllSet) :
	_index(vecBuf),
	_root(DEFAULT_ROOT),
	_clientMaxBody(CLIENT_MAX_BODY),
	_autoindex(false),
	_allowedMethods(DEFAULT),
	_set(ProgramDefaultsAllSet) {
	static const uchar m[] = DEFAULT_METHODS;
	for (int i = 0; i < DEFAULT_METHODS_LEN; i++)
		_allowedMethods |= (1 << m[i]);

	vecBuf.push_back(StrView(DEFAULT_INDEX));
	_index = Span<StrView>(vecBuf, 0, 1);
}

// Getters
const Span<StrView> &Overrides::getIndex() const { return _index; }
const StrView &Overrides::getRoot() const { return _root; }
bool Overrides::isAutoindexed() const { return _autoindex; }
size_t Overrides::getClientMaxBody() const { return _clientMaxBody; }
size_t Overrides::getErrorMapSize() const { return _error.size(); }
uchar Overrides::getAllowedMethods() const { return _allowedMethods; }

const char *Overrides::findErrorFile(uint errorCode) const {
	std::map<uint, StrView>::const_iterator it = _error.find(errorCode);
	return (it != _error.end()) ? it->second.data() : NULL;
}

uchar Overrides::isAllowedMethod(uchar methodToCheck) const {
	return _allowedMethods & (1 << methodToCheck);
}

void Overrides::inheritUnsetParams(const Overrides &parent) {
	if (!(_set & (1 << F_ROOT)) && (parent._set & (1 << F_ROOT)))
		_root = parent._root;
	if (!(_set & (1 << F_INDEX)) && (parent._set & (1 << F_INDEX)))
		_index = parent._index;
	if (!(_set & (1 << F_AUTOINDEX)) && (parent._set & (1 << F_AUTOINDEX)))
		_autoindex = parent._autoindex;
	if (!(_set & (1 << F_CLIENT_BODY)) && (parent._set & (1 << F_CLIENT_BODY)))
		_clientMaxBody = parent._clientMaxBody;
	if (!(_set & (1 << F_METHODS)) && (parent._set & (1 << F_METHODS)))
		_allowedMethods = parent._allowedMethods;

	// error pages: UNION, not overwrite — child wins per-code
	// insert does nothing if key already exists
	std::map<uint, StrView>::const_iterator it = parent._error.begin();
	for (; it != parent._error.end(); ++it)
		_error.insert(*it);

	// Add to the bits already set
	_set |= parent._set;
}

// Print
void Overrides::printMap(const char *label, ostream &stream) const {
	map<uint, StrView>::const_iterator cur = _error.begin();
	map<uint, StrView>::const_iterator end = _error.end();
	stream << label;
	if (cur == end) {
		stream << "NONE\n";
		return;
	}
	for (; cur != end; ++cur)
		stream << cur->first << ": " << cur->second << '\n';
}

void Overrides::printMethods(ostream &stream) const {
	stream << "\t\tAllowed Methods (bitset: " << bitset<8>(_allowedMethods)
		   << "): ";
	if (!_allowedMethods) {
		stream << "NONE\n";
		return;
	}

	uchar count = 0;
	for (size_t method = GET; method <= DELETE; method++)
		if ((1 << method) & _allowedMethods)
			count++;

	for (size_t method = GET; method <= DELETE; method++)
		if ((1 << method) & _allowedMethods)
			stream << _methodStrs[method] << (--count ? ", " : "\n");
}

void Overrides::printOverrides(const char *label, ostream &stream) const {
	stream << label << ":\n";
	stream << "\t\tRoot: " << getRoot() << '\n';
	stream << "\t\tAutoindex: " << (isAutoindexed() ? "true" : "false") << '\n';
	stream << "\t\tClient Max Body: " << getClientMaxBody() << '\n';
	printMap("\t\tError pages: ", stream);
	stream << "\t\tIndex files (LEN:" << _index.len() << "): ";
	for (size_t i = 0; i < _index.len(); i++)
		stream << _index[i] << ", ";
	stream << '\n';
	printMethods(stream);
}
