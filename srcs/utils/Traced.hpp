#pragma once
#include <cstdarg>
#include <cstdio>

#define TRACED_BUF_SIZE 256

class Traced
{
protected:
	char buf[TRACED_BUF_SIZE];

public:
	/* Constructor */
	Traced(const char *file, int line, const char *func, const char *fmt, ...)
	{
		int prefix = std::snprintf(buf, TRACED_BUF_SIZE, "%s:%d | %s(): ", file,
								   line, func);
		if (prefix < 0)
			prefix = 0; // Called while throwing so no throw

		va_list args;
		va_start(args, fmt);
		std::vsnprintf(buf + prefix, TRACED_BUF_SIZE - prefix, fmt, args);
		va_end(args);
	}

	/* Methods */
	const char *str() const { return buf; }
};

#define TRACED(...) Traced(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__).str()
