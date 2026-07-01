#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <ctime>
#include <string>
#include <sys/time.h>

/* Timestamp formatter with millisecond precision.
 * Calls gettimeofday on every format request — not cached. */
class Clock
{
public:
	Clock();
	~Clock();

	std::string getFormatedTime(const int format);
	std::string nowDateTime();
	std::string nowDate();
	std::string nowTime();

private:
	enum e_format { TIME, DATE, DATETIME };
	time_t _unixNow;
	struct tm *_now;
	long _microseconds;

	/* Explicit disables */
	Clock &operator=(const Clock &other);
	Clock(const Clock &other);
};

#endif
