#include "Location.hpp"
#include "Http.hpp"
#include "Overrides.hpp"
#include "Span.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <bitset>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <sstream>

using std::bitset;
using std::ostream;
using std::size_t;
using std::stringstream;

const char *Location::_methodStrs[4] = {"DEFAULT", "GET", "POST", "DELETE"};

// Public constructors and destructors

Location::Location(std::vector<StrView> &vecBuf) :
	_overrides(vecBuf),
	_cgiExtensions(vecBuf),
	_cgiPath(vecBuf),
	_returnCode(0),
	_uploadEnable(false),
	_allowedMethods(DEFAULT) {}

const Overrides &Location::getOverrides() const { return _overrides; }
const char *Location::getPath() const { return _path.data(); }
const char *Location::getReturnPath() const { return _returnPath.data(); }
const char *Location::getUploadPath() const { return _uploadPath.data(); }
uint Location::getReturncode() const { return _returnCode; }
bool Location::getUploadEnabled() const { return _uploadEnable; }

const char *Location::getRewriteOldPath() const { return _rewrite_old.data(); }

const char *Location::getRewriteNewPath() const { return _rewrite_new.data(); }

bool Location::usingDefaultMethods() const {
	return (_allowedMethods == DEFAULT ? true : false);
}

uchar Location::isAllowedMethod(uchar methodToCheck) const {
	return _allowedMethods & (1 << methodToCheck);
};

// CGI
const Span<StrView> &Location::getCgiPath() const { return _cgiPath; }

const Span<StrView> &Location::getCgiExtensions() const {
	return _cgiExtensions;
}

const char *Location::findCgiPath(StrView &extention) const {
	return findCgiPath(extention.data());
}

const char *Location::findCgiPath(const char *extention) const {
	for (size_t i = 0; i < _cgiExtensions.len(); i++)
		if (_cgiExtensions[i].compare(extention))
			return _cgiPath[i].data();
	return NULL;
}

const char *Location::safeStr(const char *str) const {
	return str ? str : "NULL";
}

void Location::printStrvSpan(const char *msg, const Span<StrView> &span,
							 ostream &stream) const {
	size_t i = 0;
	stream << msg;
	for (i = 0; i < span.len(); i++)
		stream << safeStr(span[i].data()) << ", ";
	if (i == 0)
		stream << "NONE";
	stream << '\n';
}

void Location::printMethods(ostream &stream) const {
	bool none = true;
	stream << "\tAllowed Methods (bitset: " << bitset<8>(_allowedMethods)
		   << "): ";
	for (size_t method = GET; method <= DELETE; method++)
		if ((1 << method) & _allowedMethods) {
			stream << _methodStrs[method] << " ,";
			none = false;
		}
	if (none)
		stream << "NONE";
	stream << '\n';
}

void Location::printLocation(ostream &stream) const {
	printLocation(NO_INDEX, stream);
}

void Location::printLocation(ssize_t index, ostream &stream) const {
	if (DEFAULT_LOCATION == index)
		stream << "Default Location:\n";
	else if (NO_INDEX != index)
		stream << "  [" << index << "]";

	stream << " Path: " << safeStr(getPath()) << '\n';
	stream << "\tReturn Code: " << getReturncode() << '\n';
	stream << "\tReturn Path: " << getReturnPath() << '\n';
	stream << "\tUpload Enabled: " << (getUploadEnabled() ? "true" : "false")
		   << '\n';
	stream << "\tUpload Path: " << safeStr(getUploadPath()) << '\n';

	printStrvSpan("\tCGI Extensions: ", _cgiExtensions, stream);
	printStrvSpan("\tCGI Paths: ", _cgiPath, stream);

	printMethods(stream);

	_overrides.printOverrides("\tOverrrides", stream);
}
