#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Overrides.hpp"
#include "Span.hpp"
#include "StrView.hpp"
#include <ostream>
#include <vector>

#define DEFAULT_LOCATION -1
#define NO_INDEX -2

struct Location {
public:
	// Construnctor
	Location(std::vector<StrView> &vecBuf);
	Location();

	static const char *_methodStrs[4];
	// Substructs
	Overrides _overrides;
	// Member vars
	Span<StrView> _cgiExtensions;
	Span<StrView> _cgiPath;
	StrView _path;
	StrView _returnPath;
	StrView _rewrite_old;
	StrView _rewrite_new;
	StrView _uploadPath;
	uint _returnCode;
	bool _uploadEnable;
	uchar _allowedMethods;
	// Getters Location Vars
	bool usingDefaultMethods() const;
	uchar isAllowedMethod(uchar methodToCheck) const;
	const char *findCgiPath(StrView &extention) const;
	const char *findCgiPath(const char *extention) const;
	const char *getPath() const;
	const char *getUploadPath() const;
	const char *getRewriteNewPath() const;
	const char *getRewriteOldPath() const;
	const char *getReturnPath() const;
	uint getReturncode() const;
	bool getUploadEnabled() const;
	const Span<StrView> &getCgiExtensions() const;
	const Span<StrView> &getCgiPath() const;
	const Overrides &getOverrides() const;
	// Mathods
	const char *safeStr(const char *str) const;
	void printMethods(std::ostream &stream) const;
	void printLocation(std::ostream &stream) const;
	void printLocation(ssize_t index, std::ostream &stream) const;
	void printStrvSpan(const char *msg, const Span<StrView> &span,
					   std::ostream &stream) const;
};

inline std::ostream &operator<<(std::ostream &os, const Location &location) {
	location.printLocation(os);
	return os;
}

#endif
