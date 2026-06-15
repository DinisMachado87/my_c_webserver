#include "Overrides.hpp"
#include "Span.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

using std::map;
using std::ostream;
using std::string;
using std::stringstream;
using std::vector;

Overrides::Overrides(std::vector<StrView> &vecBuf) :
	_index(vecBuf),
	_clientMaxBody(0),
	_uploadMaxBody(0),
	_autoindex(false) {}

const Span<StrView> &Overrides::getIndex() const { return _index; };
const char *Overrides::getRoot() const { return _root.data(); };
bool Overrides::isAutoindexed() const { return _autoindex; };
size_t Overrides::getClientMaxBody() const { return _clientMaxBody; };
size_t Overrides::getErrorMapSize() const { return _error.size(); };

const char *Overrides::findErrorFile(uint errorCode) const {
	std::map<uint, StrView>::const_iterator it = _error.find(errorCode);
	return ((it != _error.end()) ? it->second.data() : NULL);
};

const char *Overrides::safeStr(const char *str) const {
	return str ? str : "NULL";
}

void Overrides::printMap(const char *label, ostream &stream) const {
	map<uint, StrView>::const_iterator curError = _error.begin();
	map<uint, StrView>::const_iterator end = _error.end();

	stream << label;
	if (curError == end) {
		stream << "NONE\n";
		return;
	}
	for (; curError != end; curError++)
		stream << curError->first << ": " << safeStr(curError->second.data())
			   << '\n';
}

void Overrides::printOverrides(const char *label, ostream &stream) const {
	stream << label << ":" << '\n';
	stream << "\t\tRoot: " << safeStr(getRoot()) << '\n';
	stream << "\t\tAutoindex: " << (isAutoindexed() ? "true" : "false") << '\n';
	stream << "\t\tClient Max Body: " << getClientMaxBody() << '\n';
	stream << "\t\tClient Upload Body: " << getClientMaxBody() << '\n';
	printMap("\t\tError pages: ", stream);

	stream << "\t\tIndex files (LEN:" << _index.len() << "):";
	for (size_t i = 0; i < _index.len(); i++)
		stream << safeStr(_index[i].data()) << ", ";
	stream << "\n\n";
}
