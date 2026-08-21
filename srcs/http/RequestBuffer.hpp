#pragma once
#include "IBuffer.hpp"

/* Holds one active segment,
 * hands the parser its unparsed view,
 * copies the leftover chars to a new segment when the active segment fills.
 * Retains filled segments — parsed StrViews point into them until Request
 * destructor. Never drains, so retention is safe. */
class RequestBuffer : public IBuffer
{
public:
	static const ssize_t CARRY_OVERFLOW = -2; // token larger than a segment

	RequestBuffer(const Reader &reader, BufferManager &pool);
	virtual ~RequestBuffer();

	virtual ssize_t readIn(); // fills _active; relocates carry when full
	StrView unparsed() const; // _active->unusedView(): carry + new
	void consumed(size_t n);  // parser reports bytes eaten

private:
	Segment *_active;

	bool moveLeftoverToNewSegment(); // false when carry >= RECV_SIZE

	RequestBuffer();
	RequestBuffer(const RequestBuffer &other);
	RequestBuffer &operator=(const RequestBuffer &other);
};
