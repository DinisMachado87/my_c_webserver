#ifndef SPAN_H
#define SPAN_H

#include "webServ.hpp"
#include <ostream>
#include <stdexcept>
#include <vector>

/* Non-owning view into a subrange of a std::vector.
 * Bounds-checked access via operator[]. */
template <typename T> class Span
{
private:
	std::vector<T> *_vecBuf;
	uchar _offset;
	uchar _len;

public:
	Span(std::vector<T> &vecBuf) :
		_vecBuf(&vecBuf),
		_offset(0),
		_len(0)
	{
	}

	Span(std::vector<T> &vecBuf, uint offset, uchar len) :
		_vecBuf(&vecBuf),
		_offset(offset),
		_len(len)
	{
	}

	Span(const Span &other) :
		_vecBuf(other._vecBuf),
		_offset(other._offset),
		_len(other._len)
	{
	}

	~Span() {}

	Span &operator=(const Span &other)
	{
		if (this == &other)
			return *this;

		_vecBuf = other._vecBuf;
		_offset = other._offset;
		_len = other._len;
		return *this;
	}

	// Non-const delegates to const to avoid duplicating bounds check.
	T &operator[](uint i)
	{
		return const_cast<T &>(static_cast<const Span &>(*this)[i]);
	}

	const T &operator[](uint i) const
	{
		if (i >= _len)
			throw std::out_of_range("Span index out of range");
		return (*_vecBuf)[_offset + i];
	}

	size_t len() const { return _len; }
};

template <typename T>
std::ostream &operator<<(std::ostream &stream, const Span<T> &span)
{
	size_t i = 0;
	for (; i < span.len(); i++) {
		bool isLast = (i + 1 == span.len());
		stream << span[i] << ((!isLast) ? ", " : "");
	}
	return stream;
}

#endif
