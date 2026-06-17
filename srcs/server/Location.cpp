#include "Location.hpp"
#include "Overrides.hpp"
#include "Span.hpp"
#include "StrView.hpp"
#include "webServ.hpp"
#include <cstddef>
#include <ostream>

using std::ostream;
using std::size_t;

Location::Location(std::vector<StrView> &vecBuf) :
	_overrides(vecBuf),
	_cgiExtensions(vecBuf),
	_cgiPath(vecBuf),
	_returnCode(0),
	_uploadEnable(false) {}

// Getters
const Overrides &Location::getOverrides() const { return _overrides; }
const StrView &Location::getPath() const { return _path; }
const StrView &Location::getReturnPath() const { return _returnPath; }
const StrView &Location::getUploadPath() const { return _uploadPath; }
const StrView &Location::getRewriteOldPath() const { return _rewrite_old; }
const StrView &Location::getRewriteNewPath() const { return _rewrite_new; }
uint Location::getReturncode() const { return _returnCode; }
bool Location::getUploadEnabled() const { return _uploadEnable; }

const Span<StrView> &Location::getCgiPath() const { return _cgiPath; }
const Span<StrView> &Location::getCgiExtensions() const {
	return _cgiExtensions;
}

const char *Location::findCgiPath(StrView &extension) const {
	return findCgiPath(extension.data());
}

const char *Location::findCgiPath(const char *extension) const {
	for (size_t i = 0; i < _cgiExtensions.len(); i++)
		if (_cgiExtensions[i].compare(extension)) return _cgiPath[i].data();
	return NULL;
}

// Print
void Location::printStrvSpan(const char *msg, const Span<StrView> &span,
							 ostream &stream) const {
	size_t i = 0;
	stream << msg;
	for (i = 0; i < span.len(); i++)
		stream << span[i].data() << ", ";
	if (i == 0) stream << "NONE";
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

	stream << " Path: " << getPath() << '\n';
	stream << "\tReturn Code: " << getReturncode() << '\n';
	stream << "\tReturn Path: " << getReturnPath() << '\n';
	stream << "\tUpload Enabled: " << (getUploadEnabled() ? "true" : "false")
		   << '\n';
	stream << "\tUpload Path: " << getUploadPath() << '\n';

	printStrvSpan("\tCGI Extensions: ", _cgiExtensions, stream);
	printStrvSpan("\tCGI Paths: ", _cgiPath, stream);

	_overrides.printOverrides("\tOverrides", stream);
}
