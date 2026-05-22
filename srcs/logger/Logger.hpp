#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "Clock.hpp"
#include "webServ.hpp"
#include <climits>
#include <cstddef>
#include <ctime>
#include <fstream>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>

class Server;

class Logger {
	// STATIC SINGLETON
public:
	static Logger *_loggerPtr;
	static Logger *logger();
	static void deleteLogger();

private:
	static const char *_labels[];

	// INTANCE BASED
public:
	typedef enum {
		NONE,
		ERROR,
		WARNING,
		DEBUG,
		TITLE,
		LOG,
		CONTENT
	} e_logLevel;

	// Methods
	void log(const int level, const char *label, const char *msg,
			 size_t len = 0, const int num = NONUM, const int socket = 0,
			 in_addr_t host = INT_MAX);
	void logNumArr(const int level, const char *label, const char *msg,
				   const int *nums, size_t numCount, const int socket);
	void logError(const char *label, const std::runtime_error &errorMsg,
				  const int socket = 0);
	std::string traced(const char *msg, const char *file, const int line,
					   const char *func);
	void logTitle(const char *msg);
	void logServer(const char *msg, const Server &server);

	template <typename T>
	void logObj(const char *msg, const T *obj,
				void (T::*method)(std::ostream &) const) {
		if (CONTENT > _level)
			return;
		std::stringstream stream;
		stream << msg << '\n';
		(obj->*method)(stream);
		print(CONTENT, stream);
	}

	template <typename T> void logObj(const char *msg, const T &obj) {
		if (CONTENT > _level)
			return;
		std::stringstream stream;
		stream << msg << '\n' << obj;
		print(CONTENT, stream);
	}

private:
	e_logLevel _level;
	Clock _clock;
	std::ofstream _logFile;
	static const char *_color[];
	// explicit disables
	Logger();
	Logger(const Logger &other);
	~Logger();
	Logger &operator=(const Logger &other);

	// Private Methods
	void print(const int level, std::stringstream &stream);
	void addHost(std::stringstream &stream, in_addr_t host);
	void buildLogPrefix(std::stringstream &stream, const int level,
						const char *label, const int socket, in_addr_t host);
	void appendMessage(std::stringstream &stream, const char *msg, size_t len);
};

#ifdef LOGGING
// Core
#define LOG_INTERNAL(level, label, msg, len, num, socket, host)                \
	Logger::logger()->log(level, label, msg, len, num, socket, host)
#define LOGNUMARR_INTERNAL(level, label, msg, nums, count, socket)             \
	Logger::logger()->logNumArr(level, label, msg, nums, count, socket)

// Labeled
#define LOG_LABELED(level, label, msg)                                         \
	LOG_INTERNAL(level, label, msg, 0, NONUM, 0, INT_MAX)
#define LOGNUM_LABELED(level, label, msg, num)                                 \
	LOG_INTERNAL(level, label, msg, 0, num, 0, INT_MAX)
#define LOGSOCK_LABELED(level, label, msg, sock)                               \
	LOG_INTERNAL(level, label, msg, 0, NONUM, sock, INT_MAX)
#define LOGSOCKHOST_LABELED(level, label, msg, sock, host)                     \
	LOG_INTERNAL(level, label, msg, 0, NONUM, sock, host)

// Base
#define LOG(level, msg) LOG_INTERNAL(level, NULL, msg, 0, NONUM, 0, INT_MAX)
#define LOGNUM(level, msg, num)                                                \
	LOG_INTERNAL(level, NULL, msg, 0, num, 0, INT_MAX)
#define LOGSOCK(level, msg, socket)                                            \
	LOG_INTERNAL(level, NULL, msg, 0, NONUM, socket, INT_MAX)
#define LOGSOCKNUM(level, msg, num, socket)                                    \
	LOG_INTERNAL(level, NULL, msg, 0, num, socket, INT_MAX)
#define LOGSOCKHOST(level, msg, port, host, socket)                            \
	LOG_INTERNAL(level, NULL, msg, 0, port, socket, host)

// TRUNC
#define LOGTRUNC(level, msg, len)                                              \
	LOG_INTERNAL(level, NULL, msg, len, NONUM, 0, INT_MAX)

// Num Pair
#define LOGSOCKNUM2_LABELED(level, label, msg, v1, v2, socket)                 \
	do {                                                                       \
		int _arr[] = {v1, v2};                                                 \
		LOGNUMARR_INTERNAL(level, label, msg, _arr, 2, socket);                \
	} while (0)

#define LOGNUMS2(level, msg, v1, v2)                                           \
	LOGSOCKNUM2_LABELED(level, NULL, msg, v1, v2, 0)

#define LOGSOCKNUMS2(level, msg, v1, v2, socket)                               \
	LOGSOCKNUM2_LABELED(level, NULL, msg, v1, v2, socket)

#define LOGNUMARR2_LABELED(level, label, msg, v1, v2)                          \
	LOGSOCKNUM2_LABELED(level, label, msg, v1, v2, 0)

// Error
#define LOG_ERROR_SOCK_LABELED(label, errMsg, socket)                          \
	Logger::logger()->logError(label, errMsg, socket)
#define LOG_ERROR_LABELED(label, errMsg)                                       \
	LOG_ERROR_SOCK_LABELED(label, errMsg, 0)
#define LOG_ERROR_SOCK(errMsg, socket)                                         \
	LOG_ERROR_SOCK_LABELED(NULL, errMsg, socket)
#define LOG_ERROR(errMsg) LOG_ERROR_SOCK_LABELED(NULL, errMsg, 0)

// special
#define LOG_TITLE(msg) Logger::logger()->logTitle(msg)
#define LOG_SERVER(msg, server) Logger::logger()->logServer(msg, server)
#define LOG_OBJ(msg, obj) Logger::logger()->logObj(msg, obj)
#define LOG_OBJ_FUNC(msg, obj, func) Logger::logger()->logObj(msg, obj, func)
#define TRACED(msg)                                                            \
	Logger::logger()->traced(msg, __FILE__, __LINE__, __FUNCTION__)

#else
#define LOG_INTERNAL(level, label, msg, len, num, socket, host) (void)0
#define LOGNUMARR_INTERNAL(level, label, msg, nums, count, socket) (void)0
#define LOG_LABELED(level, label, msg) (void)0
#define LOGNUM_LABELED(level, label, msg, num) (void)0
#define LOGSOCK_LABELED(level, label, msg, sock) (void)0
#define LOGSOCKHOST_LABELED(level, label, msg, sock, host) (void)0
#define LOG(level, msg) (void)0
#define LOGTRUNC(level, msg, len) (void)0
#define LOGNUM(level, msg, num) (void)0
#define LOGTRUNCNUM(level, msg, len, num) (void)0
#define LOGSOCK(level, msg, socket) (void)0
#define LOGSOCKTRUNC(level, msg, len, socket) (void)0
#define LOGSOCKNUM(level, msg, socket, num) (void)0
#define LOGSOCKHOST(level, msg, socket, host) (void)0
#define LOG_TITLE(msg) (void)0
#define LOG_SERVER(msg, server) (void)0
#define LOG_ERROR(errMsg) (void)0
#define LOG_ERROR_SOCK(errMsg, socket) (void)0
#define LOG_ERROR_LABELED(label, errMsg) (void)0
#define LOG_ERROR_SOCK_LABELED(label, errMsg, socket) (void)0
#define TRACED(str, __FILE__, __LINE__, __FUNCTION__) (void)0
#define LOGNUMARR(level, msg, nums, count) (void)0
#define LOGSOCKNUMARR(level, msg, nums, count, socket) (void)0
#define LOGNUMARR_LABELED(level, label, msg, nums, count) (void)0
#define LOGSOCKNUMARR_LABELED(level, label, msg, nums, count, socket) (void)0
#endif
#endif
