// kore.io_websocket soup
#define MS_CLASS "Channel::UnixStreamSocket"
// #define MS_LOG_DEV

#include "Channel/UnixStreamSocket.hpp"
#include "deplibuv.hpp"
#include "Logger.hpp"
#include "MediaSoupError.hpp"
#include <cmath>   // std::ceil()
#include <cstdio>  // sprintf()
#include <cstring> // std::memmove()
#include <sstream> // std::ostringstream
#include <string>
#include <unistd.h> //usleep
#include <memory.h>
#include <string.h>

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
extern "C" {
//#include <netstring.h>
}
uv_callback_t from_cpp;
/*
void on_clo_unx(uv_handle_t*client){
printf("On_clo() occurred.\n");
}
void on_walk(uv_handle_t*handle, void * arg){
printf("on_walk unixstream socket\n");
uv_close(handle, on_clo_unx);
}
*/ 
namespace Channel
{
	/* Static. */

	// netstring length for a 65536 bytes payload.
	static constexpr size_t MaxSize{ 65543 };
	static constexpr size_t MessageMaxSize{ 65536 };
	static uint8_t WriteBuffer[MaxSize];
	
void on_clo_unx(uv_handle_t*client){
printf("On_clo() occurred.\n");
}



void on_walk(uv_handle_t*handle, void * arg){
printf("on_walk unixstream socket\n");
uv_stop(deplibuv::getloop());
uv_close(handle, on_clo_unx);
uv_run(deplibuv::getloop(),UV_RUN_DEFAULT);
}

	/* Instance methods. */

UnixStreamSocket::UnixStreamSocket(int fd) //: ::UnixStreamSocket::UnixStreamSocket(fd, MaxSize)
{
		MS_TRACE_STD();
		
		uv_loop_t*mloop=deplibuv::getloop();
		uv_loop_set_data(mloop,(void*)this);
		
		void*m22=uv_loop_get_data(mloop);
		
int rc=uv_callback_init(mloop, &to_cpp, UnixStreamSocket::on_to_cpp, UV_DEFAULT);
std::printf("uv_callback_t &to_cpp init: %d\n", rc);
if(rc !=0){

free(m22);
return;

}
//UnixStreamSocket::alisa=2;
//printf(red "Alisa in unxstrsock is %d\n" rst, UnixStreamSocket::alisa);
int rc2 = uv_callback_init(mloop,&from_cpp,on_from_cpp, UV_DEFAULT);
std::printf("uv_callback_t &from_cpp init: %d\n", rc2);
if(rc2 !=0){
free(m22);
if(rc==0)uv_walk(mloop, on_walk, NULL);	
return;
}
		
		// Create the JSON reader.
		{
			Json::CharReaderBuilder builder;
			Json::Value settings = Json::nullValue;
			Json::Value invalidSettings;

			builder.strictMode(&settings);

			MS_ASSERT(builder.validate(&invalidSettings), "invalid Json::CharReaderBuilder");

			this->jsonReader = builder.newCharReader();
		}

		// Create the JSON writer.
		{
			Json::StreamWriterBuilder builder;
			Json::Value invalidSettings;

			builder["commentStyle"]            = "None";
			builder["indentation"]             = "";
			builder["enableYAMLCompatibility"] = false;
			builder["dropNullPlaceholders"]    = false;

			MS_ASSERT(builder.validate(&invalidSettings), "invalid Json::StreamWriterBuilder");

			this->jsonWriter = builder.newStreamWriter();
		}
	}
	


void UnixStreamSocket::destroy(){
	delete this;
	printf("achieved destroy()\n");
	usleep(2000);
	// data is null, just so, let the loop to get rid of two uv_callbacks at shutdown
	int r=uv_callback_fire(&from_cpp, NULL,NULL);
	std::printf(yellow "uv_callback_t &from_cpp fire at SHUTDOWN LEVEL: %d\n" rst,r);
	uv_walk(deplibuv::getloop(), on_walk, NULL);
	}
	UnixStreamSocket::~UnixStreamSocket()
	{

MS_TRACE_STD();
		delete this->jsonReader;
		delete this->jsonWriter;
		std::printf(yellow "Look ma, ~UnixStreamSocket() destructor!\n" rst);
	
	}

void UnixStreamSocket::Soup_Shutdown(int asig){
printf("Soup::Shutdown()\n");
void*bu=uv_loop_get_data(deplibuv::getloop());
if(asig==1){	
static_cast<UnixStreamSocket*>(bu)->destroy();
return;
}
static_cast<UnixStreamSocket*>(bu)->about_soup_ending();
}
		
void UnixStreamSocket::about_soup_ending(){
	printf("::about_soup_ending()\n");
	this->listener->soup_ending();
}		


void * UnixStreamSocket::on_to_cpp(uv_callback_t*callback,void*data)
{
	if(data==NULL)return nullptr;
//std::printf("uv_callback_t UnixStreamSocket::on_to_cpp occured!: %s\n",(char*)data);
std::printf(yellow "uv_callback_t UnixStreamSocket::on_to_cpp occured!\n" rst);
	//char * loop_data=(char*)((uv_handle_t*)callback)->loop->data;
void*loop_data=uv_loop_get_data(deplibuv::getloop());
	//static_cast<UnixStreamSocket*>(loop_data)->UserOnUnixStreamRead("{\"dama\":\"sama\"}\0");
	static_cast<UnixStreamSocket*>(loop_data)->UserOnUnixStreamRead(data);
return nullptr;
}

void UnixStreamSocket::SetListener(Listener* listener)
	{
		
		MS_TRACE_STD();
		
		std::printf("Entering UnixStreamSocket::SetListener(listener)\n");
this->listener = listener;
	}

	void UnixStreamSocket::Send(Json::Value& msg)
	{
		//return;
		std::printf("Entering UnixStreamSocket::Send(Json)\n");
		
	//	std::cout << msg << std::endl;

		 MS_TRACE_STD();

		std::ostringstream stream;
		size_t nsPayloadLen;
		size_t nsNumLen;
		size_t nsLen;
//return;
		this->jsonWriter->write(msg, &stream);
	
		const std::string tmp=stream.str();
		char*wl=strdup(tmp.c_str());
		
		printf("WL: %s\n", wl);
		
		
		int r=uv_callback_fire(&from_cpp, wl,NULL);
		std::printf("uv_callback_t &from_cpp fire: %d\n",r);
		
/*
		if (nsPayloadLen > MessageMaxSize)
		{
			std::printf("mesage too big*************************************************************************************\n");

			//return;
		}
		*/ 
		
	}
	
void UnixStreamSocket::SendLog(char* nsPayload, size_t nsPayloadLen)
	{
		
UnixStreamSocket::alisa=2;
printf(red "Alisa in ::SendLog is %d\n" rst, UnixStreamSocket::alisa);
		
		//return;
		//if(suchara==1){printf(red "yes\n" rst);return;}else{printf(red "no\n" rst);}
		std::printf("SENDLOG: %s\n",nsPayload);
		
		return;
		
		char*wl=strdup(nsPayload);
		int r=uv_callback_fire(&from_cpp, wl, NULL);
		std::printf("uv_callback_t &from_cpp fire: %d\n",r);
	}

	void UnixStreamSocket::SendBinary(const uint8_t* nsPayload, size_t nsPayloadLen)
	{
std::printf("Entering UnixStreamSocket::SendBinary(const uint8_t* nsPayload, size_t nsPayloadLen).\n");

	}

void UnixStreamSocket::UserOnUnixStreamRead(void*data)
	{

MS_TRACE_STD();
//std::printf("Entering UnixStreamSocket::UserOnUnixStreamRead() %s\n",(char*)data);
std::printf(yellow "Entering UnixStreamSocket::UserOnUnixStreamRead()\n" rst);
// Be ready to parse more than a single message in a single TCP chunk.
//std::string text="{\"mama\":\"papa\"}";
std::string text2=(char*)data;
free(data);
data=NULL;
		
	//	while (true){
			//if (IsClosing())return;

//size_t readLen  = this->bufferDataLen - this->msgStart;
char* jsonStart = nullptr;
size_t jsonLen;

			// If here it means that jsonStart points to the beginning of a JSON string
			// with jsonLen bytes length, so recalculate readLen.
			size_t readLen = 8;//reinterpret_cast<const uint8_t*>(jsonStart) - (/*this->buffer + */this->msgStart);
			          //jsonLen + 1;

			Json::Value json;
			std::string jsonParseError;

			//if (this->jsonReader->parse((const char*)0, (const char*)10, (Json::Value*)"k", &jsonParseError))
			if(this->jsonReader->parse(text2.c_str(),text2.c_str()+text2.size(),&json,&jsonParseError))
			{
				std::printf("After json parsing.\n");
				//std::cout << json << std::endl;
				Channel::Request* request = nullptr;

				try
				{
					std::printf("Creating ::Request.\n");
					request = new Channel::Request(this, json);
				}
				catch (const MediaSoupError& error)
				{
					printf(red " discarding wrong channel request\n");
					MS_ERROR_STD("discarding wrong Channel request\n" rst);
				}

				if (request != nullptr)
				{
					std::printf("Request is not nullptr\n");
					// Notify the listener.
					this->listener->OnChannelRequest(this, request);
					//this->listener->mfuck();

					std::printf("Deleting the Request.\n");
					delete request;
				}
			}
			else
			{
				MS_ERROR_STD("JSON parsing error: %s", jsonParseError.c_str());
			}
		
		
	}

	

	
	
	void UnixStreamSocket::UserOnUnixStreamSocketClosed(bool isClosedByPeer)
	{
		MS_TRACE_STD();
std::printf("Entering ::UserOnUnixStreamSocketClosed(bool).\n");
		this->closed = true;

		if (isClosedByPeer)
		{
			// Notify the listener.
			this->listener->OnChannelUnixStreamSocketRemotelyClosed(this);
		}
	}
} // namespace Channel

// C wrapper
void* set_channel(){
auto* chl = new Channel::UnixStreamSocket(3);
if(chl){printf(green "CHANNEL!\n" rst);}else{printf(red "no channel!\n" rst);}
return chl;


//return chl;
}
void soup_shutdown(int asig){
	printf("soup_shutdown()\n");
	Channel::UnixStreamSocket::Soup_Shutdown(asig);
	}
	
	void soup_destroy(){
	//Channel::UnixStreamSocket::destroy();	
	/*
	int r=uv_callback_fire(&from_cpp, NULL,NULL);
	std::printf(yellow "uv_callback_t &from_cpp fire at SHUTDOWN LEVEL: %d\n" rst,r);
	uv_walk(deplibuv::getloop(), on_walk, NULL);
	//delete this;
	usleep(1000);
	*/ 
	}
