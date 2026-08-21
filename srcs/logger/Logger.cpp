#include "Logger.hpp"
#include "Server.hpp"
#include <arpa/inet.h>
#include <climits>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <istream>
#include <netinet/in.h>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>

#define COLOR_DEBUG "\033[36m"
#define COLOR_INFO "\033[32m"
#define COLOR_WARN "\033[33m"
#define COLOR_ERROR "\033[31m"
#define COLOR_PURPLE "\033[35m"
#define COLOR_RESET "\033[0m"
#define SEPARATOR " | "

using std::cerr;
using std::cout;
using std::endl;
using std::ios;
using std::iostream;
using std::runtime_error;
using std::string;
using std::stringstream;

Logger *Logger::_loggerPtr = NULL;
const char *Logger::_labels[] = {"[NONE]",	"[ERROR]", "[WARNING]", "[DEBUG]",
								 "[TITLE]", "[LOG]",   "[CONTENT]"};
const char *Logger::_color[]
	= {NULL, COLOR_ERROR, COLOR_WARN, NULL, COLOR_PURPLE, COLOR_INFO, NULL};

// Public constructors and destructors
Logger::Logger() :
	_level(LOGLEVEL)
{
}

Logger::~Logger() { _logFile.close(); }

void Logger::deleteLogger()
{
	delete _loggerPtr;
	_loggerPtr = NULL;
	const char *msg = "Deleted logger instance.\n";
	write(1, msg, strlen(msg));
}

// Public Methods
Logger *Logger::logger()
{
	if (_loggerPtr)
		return _loggerPtr;

	_loggerPtr = new Logger;
	if (!_loggerPtr)
		throw runtime_error("Error creating logger instance");

	LOG(LOG, "Created Logger instance");
	_loggerPtr->_logFile.open("serverLog.txt", iostream::out);

	if (!_loggerPtr->_logFile.is_open())
		throw runtime_error("Error opening logfile");
	return _loggerPtr;
}

void Logger::print(const int level, stringstream &stream)
{
	string str = stream.str();
	const char *color = _color[level];
	if (LOGTOCLI) {
		if (level == ERROR)
			cerr << color << str << COLOR_RESET << endl;
		else if (color)
			cout << _color[level] << str << COLOR_RESET << endl;
		else
			cout << str << endl;
	}
	if (LOGTOFILE)
		_logFile << str << endl;
}

void Logger::addHost(stringstream &stream, in_addr_t host)
{
	stream << " | Host: ";
	uchar *octet = (uchar *)&host;
	stream << (int)octet[0] << '.' << (int)octet[1] << '.' << (int)octet[2]
		   << '.' << (int)octet[3];
}

void Logger::logError(const char *label, const runtime_error &errorMsg,
					  const int socket)
{
	if (ERROR > _level)
		return;

	stringstream msg;

	if (label)
		msg << label << ":\n\t";
	msg << errorMsg.what();

	log(ERROR, label, msg.str().c_str(), 0, NONUM, socket, INT_MAX);
}

void Logger::buildLogPrefix(stringstream &stream, const int level,
							const char *label, const int socket, in_addr_t host)
{
	stream << _clock.nowTime() << SEPARATOR;
	stream << _labels[level];

	if (label)
		stream << SEPARATOR << label;
	if (socket)
		stream << " | Socket: " << socket;
	if (host != INT_MAX)
		addHost(stream, host);
	stream << SEPARATOR;
	if (level == CONTENT)
		stream << '\n';
}

void Logger::appendMessage(stringstream &stream, const char *msg, size_t len)
{
	if (len)
		stream.write(msg, len);
	else
		stream << msg;
}

void Logger::log(const int level, const char *label, const char *msg,
				 size_t len, const int num, const int socket, in_addr_t host)
{
	if (level > _level)
		return;

	stringstream stream;
	buildLogPrefix(stream, level, label, socket, host);
	appendMessage(stream, msg, len);
	if (num != NONUM)
		stream << num;

	print(level, stream);
}

void Logger::logNumArr(const int level, const char *label, const char *msg,
					   const int *nums, size_t numCount, const int socket)
{
	if (level > _level)
		return;

	stringstream stream;
	buildLogPrefix(stream, level, label, socket, INT_MAX);
	stream << msg;

	if (nums && numCount > 0) {
		for (size_t i = 0; i < numCount; ++i) {
			if (i > 0)
				stream << ", ";
			stream << nums[i];
		}
	}
	print(level, stream);
}

void Logger::logTitle(const char *msg)
{
	if (LOG > _level)
		return;
	stringstream stream;
	stream << "=====" << msg << "=====";
	print(TITLE, stream);
}

void Logger::logServer(const char *msg, const Server &server)
{
	if (CONTENT > _level)
		return;

	stringstream stream;
	stream << msg << '\n';
	server.getServerStr(stream);
	print(CONTENT, stream);
}
