#define MS_CLASS "main"
// #define MS_LOG_DEV
#include "uv_callback.h"

#include "common.hpp"
#include "DepLibSRTP.hpp"
#include "DepLibUV.hpp"
#include "DepOpenSSL.hpp"
#include "Logger.hpp"
#include "Loop.hpp"
#include "MediaSoupError.hpp"
#include "Settings.hpp"
#include "Utils.hpp"
#include "Channel/UnixStreamSocket.hpp"
#include "RTC/DtlsTransport.hpp"
#include "RTC/SrtpSession.hpp"
#include "RTC/TcpServer.hpp"
#include "RTC/UdpSocket.hpp"
#include <uv.h>
#include <cerrno>
#include <csignal>  // sigaction()
#include <cstdlib>  // std::_Exit(), std::genenv()
#include <iostream> // std::cout, std::cerr, std::endl
#include <map>
#include <string>
#include <unistd.h> // getpid(), usleep()
uv_callback_t to_cpp;

static void init();
static void ignoreSignals();
static void destroy();
static void exitSuccess();
static void exitWithError();
void*on_from_cpp(uv_callback_t*,void*);
void*on_result(uv_callback_t*,void*);
void*on_from_cpp(uv_callback_t*,void*data){
std::printf("ON_FROM_CPP: %s\n",(char*)data);
if(!strcmp((char*)data,"exit")){
uv_stop(DepLibUV::GetLoop());
	return nullptr;
}
free(data);
data=nullptr;
return (void*)"mli";
}
void*on_result(uv_callback_t*cb,void*data){
std::printf("on result occured.\n");

return NULL;
}
const char*room_create_str="{\"id\":3444444333,\"method\":\"worker.createRoom\",\"internal\":{\"roomId\":35,\"sister\":\"sister_1\"},\"data\":{\"a\":1}}";

int main(int argc, char* argv[])
{
	// Ensure we are called by our Node library.
	/*
	if (argc == 1 || (std::getenv("MEDIASOUP_CHANNEL_FD") == nullptr))
	{
		std::cerr << "ERROR: you don't seem to be my real father" << std::endl;

		std::_Exit(EXIT_FAILURE);
	}
*/
	std::string id = "345678";//std::string(argv[1]);
	int channelFd  = 3;//std::stoi(std::getenv("MEDIASOUP_CHANNEL_FD"));

	// Initialize libuv stuff (we need it for the Channel).
	DepLibUV::ClassInit();

	// Set the Channel socket (this will be handled and deleted by the Loop).
	auto* channel = new Channel::UnixStreamSocket(channelFd);
//	void set_loop_channel(void*fi)

	// Initialize the Logger.
	Logger::Init(id, channel);

	// Setup the configuration.
	try
	{
		Settings::SetConfiguration(argc, argv);
	}
	catch (const MediaSoupError& error)
	{
		MS_ERROR("configuration error: %s", error.what());

		exitWithError();
	}

	// Print the effective configuration.
	Settings::PrintConfiguration();

	MS_DEBUG_TAG(info, "starting mediasoup-worker [pid:%ld]", (long)getpid());

#if defined(MS_LITTLE_ENDIAN)
	MS_DEBUG_TAG(info, "Little-Endian CPU detected");
#elif defined(MS_BIG_ENDIAN)
	MS_DEBUG_TAG(info, "Big-Endian CPU detected");
#endif

#if defined(INTPTR_MAX) && defined(INT32_MAX) && (INTPTR_MAX == INT32_MAX)
	MS_DEBUG_TAG(info, "32 bits architecture detected");
#elif defined(INTPTR_MAX) && defined(INT64_MAX) && (INTPTR_MAX == INT64_MAX)
	MS_DEBUG_TAG(info, "64 bits architecture detected");
#else
	MS_WARN_TAG(info, "can not determine whether the architecture is 32 or 64 bits");
#endif

	try
	{
		init();

		// Run the Loop.
	//	int r=uv_callback_fire(&to_cpp,(void*)room_create_str, NULL);
//std::printf("uv_callback_t &to_cpp fire %d\n",r);
		Loop loop(channel);

		// Loop ended.
		destroy();
		exitSuccess();
	}
	catch (const MediaSoupError& error)
	{
		MS_ERROR_STD("failure exit: %s", error.what());

		destroy();
		exitWithError();
	}
}

void init()
{
	MS_TRACE();

	ignoreSignals();
	DepLibUV::PrintVersion();

	// Initialize static stuff.
	DepOpenSSL::ClassInit();
	DepLibSRTP::ClassInit();
	
	Utils::Crypto::ClassInit();
	
	RTC::UdpSocket::ClassInit();
	RTC::TcpServer::ClassInit();
	
	RTC::DtlsTransport::ClassInit();
	
	RTC::SrtpSession::ClassInit();
	RTC::Room::ClassInit();
}

void ignoreSignals()
{
	MS_TRACE();

	int err;
	// clang-format off
	struct sigaction act{};
	std::map<std::string, int> ignoredSignals =
	{
		{ "PIPE", SIGPIPE },
		{ "HUP",  SIGHUP  },
		{ "ALRM", SIGALRM },
		{ "USR1", SIGUSR2 },
		{ "USR2", SIGUSR1}
	};
	// clang-format on

	// Ignore clang-tidy cppcoreguidelines-pro-type-cstyle-cast.
	act.sa_handler = SIG_IGN; // NOLINT
	act.sa_flags   = 0;
	err            = sigfillset(&act.sa_mask);
	if (err != 0)
		MS_THROW_ERROR("sigfillset() failed: %s", std::strerror(errno));

	for (auto& ignoredSignal : ignoredSignals)
	{
		auto& sigName = ignoredSignal.first;
		int sigId     = ignoredSignal.second;

		err = sigaction(sigId, &act, nullptr);
		if (err != 0)
		{
			MS_THROW_ERROR("sigaction() failed for signal %s: %s", sigName.c_str(), std::strerror(errno));
		}
	}
}

void destroy()
{
	MS_TRACE();

	// Free static stuff.
	RTC::DtlsTransport::ClassDestroy();
	Utils::Crypto::ClassDestroy();
	DepLibUV::ClassDestroy();
	DepOpenSSL::ClassDestroy();
	DepLibSRTP::ClassDestroy();
}

void exitSuccess()
{
	// Wait a bit so peding messages to stdout/Channel arrive to the main process.
	usleep(100000);
	// And exit with success status.
	std::_Exit(EXIT_SUCCESS);
}

void exitWithError()
{
	// Wait a bit so peding messages to stderr arrive to the main process.
	usleep(100000);
	// And exit with error status.
	std::_Exit(EXIT_FAILURE);
}
