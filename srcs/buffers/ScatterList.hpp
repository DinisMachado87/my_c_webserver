#pragma once
#include "StrView.hpp"
#include <cassert>
#include <cstddef>
#include <sys/uio.h>

/* Fixed-capacity iovec builder for scatter-gather sends. Sections point
 * into caller memory that must outlive every send. Build once with add(),
 * send via iov()/count(); on short write advance(sent) trims sent sections
 * and offsets the partial one to resume. done() when fully drained. */
template <int CAPACITY> class ScatterList
{
private:
	/* State */
	struct iovec _iovSections[CAPACITY];
	int _loadedSections;

public:
	/* Constructors */
	ScatterList() :
		_loadedSections(0)
	{
	}

	/* Methods */
	void reset() { _loadedSections = 0; }

	// Appends one span. Ignored past capacity.
	void add(const StrView &section)
	{
		if (_loadedSections >= CAPACITY || section.size() == 0)
			return;
		_iovSections[_loadedSections].iov_base
			= const_cast<char *>(section.data());
		_iovSections[_loadedSections].iov_len = section.size();
		_loadedSections++;
	}

	// Records bytes sent, trims fully-sent sections, offsets the partial one.
	void advance(size_t sent)
	{
		if (sent == 0)
			return;

		struct iovec *curSection = _iovSections;
		struct iovec *end = _iovSections + _loadedSections;
		while (curSection < end && sent >= curSection->iov_len) {
			sent -= curSection->iov_len;
			curSection++;
		}
		assert(sent == 0 || curSection < end);

		_loadedSections -= static_cast<int>(curSection - _iovSections);
		if (_loadedSections == 0)
			return;

		curSection->iov_base = static_cast<char *>(curSection->iov_base) + sent;
		curSection->iov_len -= sent;
		for (int i = 0; i < _loadedSections; i++)
			_iovSections[i] = curSection[i];
	}

	const struct iovec *iov() const { return _iovSections; }
	int loadedSections() const { return _loadedSections; }
};
