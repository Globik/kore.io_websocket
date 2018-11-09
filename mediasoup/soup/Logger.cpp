#define MS_CLASS "Logger"
// #define MS_LOG_DEV

#include "Logger.hpp"

/* Class variables. */

std::string Logger::id{ "unset" };
Channel::UnixStreamSocket* Logger::channel{ nullptr };
char Logger::buffer[Logger::bufferSize];

/* Class methods. */

void Logger::Init(const std::string& id, Channel::UnixStreamSocket* channel)
{
	std::printf("LOGGER_INIT!\n");
	Logger::id      = id;
	Logger::channel = channel;

	MS_TRACE();
}

void Logger::Init(const std::string& id)
{
	std::printf("LOGGER_INIT @!!\n");
	Logger::id = id;

	MS_TRACE();
}
void logger_init(char*s, void * fi){
Logger::Init(s, static_cast<Channel::UnixStreamSocket*>(fi));	
}
