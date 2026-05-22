#ifndef STRVIEW_HPP
#define STRVIEW_HPP

#include "webServ.hpp"
#include <ostream>
#include <string>
#include <vector>

#define SKIP true

class StrView {
private:
	static std::string _empty;

private:
	std::string *_rawBuffer;
	uint _offset;
	uint _len;

	// Methods
	const char *has(const char *color, const bool hasColor) const;

public:
	// Constructors and destructors
	StrView(std::string &buffer, const uint offset, const uint len);
	StrView(std::string *buffer, const uint offset, const uint len);
	StrView(std::string &buffer);
	StrView(const StrView &other);
	StrView();
	~StrView();
	// Operators overload
	StrView &operator=(const StrView &other);
	StrView &operator=(StrView &other);
	bool operator==(const StrView &other) const;
	bool operator!=(const StrView &other) const;
	bool operator!=(const char *str) const;
	bool operator==(const char *str) const;
	bool operator<(const StrView &other) const;
	// Getters
	const char *getStart() const;
	std::string getStr() const;
	uint getOffset() const;
	uint getLen() const;
	// Setters
	void setBuffer(std::string &newBuffer);
	void setStart(const char *start);
	void setLen(uint len);
	void setStartAndLen(const char *start, uint len);
	// Methods
	size_t segmentUntil(char separator, uint startOffset,
						StrView &segment) const;
	std::vector<StrView> splitBefore(const char c) const;
	StrView lastSplitBefore(const char c) const;
	void info(std::ostream &, const char item = 0,
			  const char *color = "") const;
	void nreplace(const uint startOffset, const StrView &toInsert,
				  const uint len);
	void replace(const uint startOffset, const StrView &toInsert);
	size_t find(const char c, const size_t addedOffset = 0) const;
	size_t findPosInBuffer(const char c, const size_t addedOffset = 0) const;
	void intoStream(std::ostream &stream) const;
	void streamStrV(std::stringstream &stream) const;
	void streamStrView(std::stringstream &stream);
	void printBuffer() const;
	bool ncompare(const char *str, size_t len) const;
	size_t getBufferSize() const;
	void nullTerminate();
	const char *getEnd() const;
	void trimEnd(const size_t trimSize);
	bool compare(StrView &str) const;
	bool compare(const char *str) const;
	void updateOffset(uint increase);
	void printStrV() const;
	void move(std::string &toBuffer);
};

// Out of class operators for reverse comparison
inline bool operator==(const char *str, const StrView &sv) { return sv == str; }
inline bool operator!=(const char *str, const StrView &sv) { return sv != str; }
inline std::ostream &operator<<(std::ostream &os, const StrView &strv) {
	strv.intoStream(os);
	return os;
}

#endif
