#include "Engine.hpp"
#include "Logger.hpp"
#include "Signals.hpp"
#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

using std::ifstream;
using std::runtime_error;
using std::string;
using std::stringstream;

static string readFile(const char *filepath)
{
	ifstream file(filepath);

	if (!file.is_open())
		throw("Error opening file: ");

	stringstream buffer;
	buffer << file.rdbuf();

	file.close();
	return buffer.str();
}

int main(int argc, char **argv)
{
	try {
		if (argc < 2)
			throw runtime_error(
				"Missing Argument. Use: ./webserv <config/path>");
		else if (argc > 2)
			throw runtime_error(
				"Too many arguments. Use: ./webserv <config/path>");

		setup_signals();
		const char *configPath = argv[1];
		string config = readFile(configPath);
		Engine engine;
		engine.run(config);
	} catch (const runtime_error &err) {
		LOG_ERROR(err);
		LOG(Logger::LOG, "Exiting gracefully from Error");
		Logger::deleteLogger();
		return (1);
	}
	LOG(Logger::LOG, "Exiting gracefully");
	Logger::deleteLogger();
	return (0);
}
