#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Overrides.hpp"
#include "Span.hpp"
#include "StrView.hpp"
#include <ostream>
#include <vector>

#define DEFAULT_LOCATION -1
#define NO_INDEX -2

/* A URI path block from the config — holds its own Overrides
 * plus location-specific directives (rewrite, CGI, upload).
 * Populated by ConfParser; read-only at runtime. */
class Location
{
private:
	Overrides _overrides;
	Span<StrView> _cgiExtensions;
	Span<StrView> _cgiPath;
	StrView _path;
	StrView _returnPath;
	StrView _rewrite_old;
	StrView _rewrite_new;
	StrView _uploadPath;
	uint _returnCode;
	bool _uploadEnable;

	friend class ConfParser;
	friend class ConfParserTest;

public:
	Location(std::vector<StrView> &vecBuf);

	/* Read interface */
	const StrView &getPath() const;
	const StrView &getReturnPath() const;
	const StrView &getUploadPath() const;
	const StrView &getRewriteOldPath() const;
	const StrView &getRewriteNewPath() const;
	const StrView &getRoot() const;

	const Overrides &getOverrides() const;
	// Returns the interpreter path for extension, or NULL if not CGI.
	const char *findCgiPath(const StrView &extension) const;
	uint getReturncode() const;
	bool getUploadEnabled() const;
	bool isAllowedMethod(const uchar method) const;
	const Span<StrView> &getCgiExtensions() const;
	const Span<StrView> &getCgiPath() const;

	/* Print */
	void printLocation(std::ostream &stream) const;
	void printLocation(ssize_t index, std::ostream &stream) const;
	void printStrvSpan(const char *msg, const Span<StrView> &span,
					   std::ostream &stream) const;
};

inline std::ostream &operator<<(std::ostream &os, const Location &location)
{
	location.printLocation(os);
	return os;
}

#endif
