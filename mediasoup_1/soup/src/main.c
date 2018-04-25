#define MS_CLASS "main"
// #define MS_LOG_DEV
#include "uv_callback.h"

//#include "common.hpp"
#include "DepLibSRTP.hpp"
#include "DepLibUV.hpp"
#include "DepOpenSSL.hpp"
#include "Logger.hpp"
#include "Loop.hpp"
//#include "MediaSoupError.hpp"
#include "Settings.hpp"
#include "Utils.hpp"
#include "Channel/UnixStreamSocket.hpp"
#include "RTC/DtlsTransport.hpp"
#include "RTC/SrtpSession.hpp"
#include "RTC/TcpServer.hpp"
#include "RTC/UdpSocket.hpp"
#include <uv.h>
//#include <cerrno>
//#include <csignal>  // sigaction()
//#include <cstdlib>  // std::_Exit(), std::genenv()
//#include <iostream> // std::cout, std::cerr, std::endl
//#include <map>
//#include <string>

#include <unistd.h> // getpid(), usleep()
uv_callback_t to_cpp;

static void init();
static void ignoreSignals();
static void destroy();
static void exitSuccess();
static void exitWithError();
void*on_from_cpp(uv_callback_t*,void*);
void*on_result(uv_callback_t*,void*);
void*on_from_cpp(uv_callback_t*cb,void*data){
printf("ON_FROM_CPP: %s\n",(char*)data);
if(!strcmp((char*)data,"exit")){
uv_stop(get_loop());
return NULL;
}
free(data);
data=NULL;
return (void*)"OK";
}
void*on_result(uv_callback_t*cb,void*data){
printf("on result occured.\n");
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
	char*id = "345678";//std::string(argv[1]);
	int channelFd  = 3;//std::stoi(std::getenv("MEDIASOUP_CHANNEL_FD"));

	// Initialize libuv stuff (we need it for the Channel).
	//DepLibUV::ClassInit();
	class_init();

	// Set the Channel socket (this will be handled and deleted by the Loop).
	//auto* channel = new Channel::UnixStreamSocket(channelFd);
	void*channel=set_channel();


	// Initialize the Logger.
	//Logger::Init(id, channel);
	logger_init(id,channel);

	// Setup the configuration.
	//try
	//{
		//Settings::SetConfiguration(argc, argv);
	settings_set_configuration(argc,argv);
	//}
	//catch (const MediaSoupError& error)
	//{
	//	MS_ERROR("configuration error: %s", error.what());

	//	exitWithError();
	//}

	// Print the effective configuration.
	//Settings::PrintConfiguration();
	settings_print_configuration();

	//MS_DEBUG_TAG(info, "starting mediasoup-worker [pid:%ld]", (long)getpid());
/*
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
*/
	//try
	//{
		init();

		// Run the Loop.
	//	int r=uv_callback_fire(&to_cpp,(void*)room_create_str, NULL);
//std::printf("uv_callback_t &to_cpp fire %d\n",r);
		//Loop loop(channel);
	set_loop_channel(channel);

		// Loop ended.
		destroy();
		exitSuccess();
	//}
	//catch (const MediaSoupError& error)
	//{
	//	MS_ERROR_STD("failure exit: %s", error.what());

	//	destroy();
	//	exitWithError();
	//}
}

void init()
{
	//MS_TRACE();

	ignoreSignals();
	//DepLibUV::PrintVersion();
	deplibuv_printversion();

	// Initialize static stuff.
	//DepOpenSSL::ClassInit();
	depopenssl_class_init();
	//DepLibSRTP::ClassInit();
	deplibsrtp_class_init();
	
	//Utils::Crypto::ClassInit();
	utils_crypto_class_init();
	
	//RTC::UdpSocket::ClassInit();
rtc_udp_socket_class_init();
	//RTC::TcpServer::ClassInit();
	rtc_tcp_server_class_init();
	//RTC::DtlsTransport::ClassInit();
	rtc_dtls_transport_class_init();
	//RTC::SrtpSession::ClassInit();
	rtc_srtp_session_class_init();
	//RTC::Room::ClassInit();
	rtc_room_class_init();
}

void ignoreSignals()
{
	/*
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
	*/
}

void destroy()
{
	//MS_TRACE();

	// Free static stuff.
	//RTC::DtlsTransport::ClassDestroy();
	rtc_dtls_transport_class_destroy();
	//Utils::Crypto::ClassDestroy();
	utils_crypto_class_destroy();
	//DepLibUV::ClassDestroy();
	class_destroy();
	//DepOpenSSL::ClassDestroy();
	depopenssl_class_destroy();
	//DepLibSRTP::ClassDestroy();
	deplibsrtp_class_destroy();
}

void exitSuccess()
{
	// Wait a bit so peding messages to stdout/Channel arrive to the main process.
	usleep(100000);
	// And exit with success status.
	//std::_Exit(EXIT_SUCCESS);
	_exit(0);
}

void exitWithError()
{
	// Wait a bit so peding messages to stderr arrive to the main process.
	usleep(100000);
	// And exit with error status.
	//std::_Exit(EXIT_FAILURE);
	_exit(1);
}
