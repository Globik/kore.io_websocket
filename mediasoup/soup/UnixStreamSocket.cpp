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
extern "C" {
//#include <netstring.h>
}
uv_callback_t from_cpp;//stop_w;
//struct pupkin*mili={nullptr};
namespace Channel
{
	/* Static. */

	// netstring length for a 65536 bytes payload.
	static constexpr size_t MaxSize{ 65543 };
	static constexpr size_t MessageMaxSize{ 65536 };
	static uint8_t WriteBuffer[MaxSize];
	

	/* Instance methods. */

	UnixStreamSocket::UnixStreamSocket(int fd) //: ::UnixStreamSocket::UnixStreamSocket(fd, MaxSize)
	{
		MS_TRACE_STD();
	//mili=new pupkin;
		//this->closed=0;
		uv_loop_t*mloop=deplibuv::getloop();
		//uv_loop_set_data(mloop,(void*)"some_data");
		uv_loop_set_data(mloop,(void*)this);
		
int rc=uv_callback_init(mloop, &to_cpp, UnixStreamSocket::on_to_cpp, UV_DEFAULT);
		std::printf("uv_callback_t &to_cpp init: %d\n",rc);
rc=uv_callback_init(mloop,&from_cpp,on_from_cpp, UV_DEFAULT);
		std::printf("uv_callback_t &from_cpp init: %d\n",rc);
		//rc=uv_callback_init(mloop,&stop_w,UnixStreamSocket::close_work,UV_DEFAULT);
		//std::printf("dummy uv_callback_t stop_w init: %d\n",rc);
		
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
void on_walk(uv_handle_t*handle, void * arg){
/*
signal destroy
 24576 1
 8192 6
 24576 1
 8193 16
*/
if(((handle)->flags) !=0){
printf(" %d %d\n",(handle)->flags, handle->type);
if((handle)->flags==8192){		
uv_close(handle,NULL);
}
if((handle)->flags==24576){uv_close(handle,NULL);}
}
uv_stop(deplibuv::getloop());	
}
	UnixStreamSocket::~UnixStreamSocket()
	{
		
	
MS_TRACE_STD();
	uv_walk(deplibuv::getloop(), on_walk,NULL);
	void * motherdata=uv_loop_get_data(deplibuv::getloop());
	free(motherdata);
std::printf("Look ma, ~UnixStreamSocket() destructor!\n");

		delete this->jsonReader;
		delete this->jsonWriter;
	}

	
void * UnixStreamSocket::on_to_cpp(uv_callback_t*callback,void*data)
{
	if(data==NULL)return nullptr;
std::printf("uv_callback_t UnixStreamSocket::on_to_cpp occured!: %s\n",(char*)data);
	//char * gu=(char*)((uv_handle_t*)callback)->loop->data;
void*bu=uv_loop_get_data(deplibuv::getloop());
	//static_cast<UnixStreamSocket*>(bu)->listener->mfuck();
	//static_cast<UnixStreamSocket*>(bu)->UserOnUnixStreamRead("{\"dama\":\"sama\"}\0");
	//static_cast<UnixStreamSocket*>(bu)->UserOnUnixStreamRead(data);
	
	//for test
	
	int r=uv_callback_fire(&from_cpp,(void*)"{\"test_test_test\":\"aha\"}",NULL);
	std::printf("uv_callback_t &from_cpp fire: %d\n",r);
	
	
	//end for test
	
	
return nullptr;
}

void UnixStreamSocket::SetListener(Listener* listener)
	{
		MS_TRACE_STD();
		std::printf("Entering UnixStreamSocket::SetListener(listener)\n");
this->listener = listener;
//this->listener->mfuck();
	}

	void UnixStreamSocket::Send(Json::Value& msg)
	{
		std::printf("Entering UnixStreamSocket::Send(Json)\n");
		std::cout << msg << std::endl;
		//struct pupkin * mili=new pupkin;
		//mili->n=4;
		//int r=uv_callback_fire(&from_cpp,(void*)"HALLO from cpp!!!",NULL);
	//	int r=uv_callback_fire(&from_cpp,(void*)"msg",NULL);
	//	std::printf("uv_callback_t &from_cpp fire: %d\n",r);
		//if (this->closed)return;

		 MS_TRACE_STD();

		std::ostringstream stream;
		// std::string nsPayload;
		size_t nsPayloadLen;
		size_t nsNumLen;
		size_t nsLen;

		this->jsonWriter->write(msg, &stream);
		//nsPayload    = stream.str();
		//nsPayloadLen = nsPayload.length();
		//static 
		const std::string tmp=stream.str();
		char*wl=strdup(tmp.c_str());
		//const char*ws=tmp.c_str();
		
		//delete [] ws;
	//	ws=new char[stream.str().size()+1];
		//ws[stream.str().size()+1];
		//ws=(char*)malloc(sizeof(char)*stream.str().size()+1);
		/*
		this->me=(char*)malloc(sizeof(char)*stream.str().size()+1);
		memcpy(this->me,stream.str().c_str(),stream.str().size()+1);
		*/
		//ws=(char*)malloc(sizeof(char)*stream.str().size()+1);
		//ws=(char*)malloc(((stream.str().size())+1)*sizeof(char));
		//memcpy(ws,stream.str().c_str(),stream.str().size()+1);
		
		//strcpy(ws,stream.str().c_str());
		/*
		std::cout << "nsPayload: " << nsPayload << std::endl;
		std::cout << "nsPayloadLen: " << nsPayloadLen << std::endl;
		
		std::printf("char  nsPayload: %s\n",nsPayload.c_str());
		std::printf("len nsPayloadLen %d\n",nsPayloadLen);
		*/
		//char*d=reinterpret_cast<char*>nsPayload.c_str();
		//const char *er=nsPayload.c_str();
		//char * d=strdup(er);
		/*
		struct pupkin*mili=(struct pupkin*)malloc(sizeof(struct pupkin));
		//mili->n=0;
		//mili->suka=(void*)nsPayload.c_str();
		//std::printf("INTEGER: %d and memm: %p\n",mili->n,mili);
		mili->suka=NULL;
		//mili->suka=(char*)malloc(sizeof(mili->suka)*nsPayloadLen);
		//memcpy(mili->suka,nsPayload.c_str(),nsPayloadLen);
		mili->n=nsPayloadLen;
		mili->suka=const_cast<char*>(nsPayload.c_str());
		*/
		//mili->size=nsPayloadLen;
		
		/*
		struct pupkin mili;
		mili.suka=(void*)nsPayload.c_str();
		mili.n=20;
		mili.fuka[nsPayloadLen]='0';//(char*)nsPayload.c_str();
		//memcpy(mili.fuka,nsPayload.c_str(),nsPayloadLen);
		*/
		//int r=uv_callback_fire(&from_cpp,(void*)nsPayload.c_str(),NULL);
		//int r=uv_callback_fire(&from_cpp,(void*)ws,NULL);
		//int r=uv_callback_fire(&from_cpp,mili,NULL);
		
		int r=uv_callback_fire(&from_cpp,wl,NULL);
		std::printf("uv_callback_t &from_cpp fire: %d\n",r);
		//free(wl);
		//std::printf(" and memm_2: %p\n",ws);
		//free(ws);
		//std::printf("WSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS %s\n",ws);
		//free(this->me);
		//free(mili);
		//r=uv_callback_fire(&from_cpp,mili,NULL);
		//std::printf("uv_callback_t &from_cpp fire: %d\n",r);
		//free(mili.fuka);
		//free((void*)d);
		//free(mili);
/*
		if (nsPayloadLen > MessageMaxSize)
		{
			std::printf("mesage too big*************************************************************************************\n");

			return;
		}
		*/
/*
		if (nsPayloadLen == 0)
		{
			nsNumLen       = 1;
			WriteBuffer[0] = '0';
			WriteBuffer[1] = ':';
			WriteBuffer[2] = ',';
		}
		else
		{
			nsNumLen = static_cast<size_t>(std::ceil(std::log10(static_cast<double>(nsPayloadLen) + 1)));
			std::sprintf(reinterpret_cast<char*>(WriteBuffer), "%zu:", nsPayloadLen);
			std::memcpy(WriteBuffer + nsNumLen + 1, nsPayload.c_str(), nsPayloadLen);
			WriteBuffer[nsNumLen + nsPayloadLen + 1] = ',';
		}

		nsLen = nsNumLen + nsPayloadLen + 2;
*/
		//Write((const uint8_t*)"mama\0", 5);
	}
	
	void UnixStreamSocket::SendLog(char* nsPayload, size_t nsPayloadLen)
	{
		std::printf("Entering UnixStreamSocket::SendLog(char, size_t).\n");
		std::printf("SENDLOG: %s\n",nsPayload);
		int r=uv_callback_fire(&from_cpp,(void*)nsPayload,NULL);
		//int r=uv_callback_fire(&from_cpp,mili,NULL);
		std::printf("uv_callback_t &from_cpp fire: %d\n",r);
	//this->closed=true;
		/*if (this->closed){
		std::printf("send log is closed.\n");
			//return;
		}
		*/
		

		// MS_TRACE_STD();
/*
		size_t nsNumLen;
		size_t nsLen;

		if (nsPayloadLen > MessageMaxSize)
		{
			MS_ERROR_STD("mesage too big");

			return;
		}

		if (nsPayloadLen == 0)
		{
			nsNumLen       = 1;
			WriteBuffer[0] = '0';
			WriteBuffer[1] = ':';
			WriteBuffer[2] = ',';
		}
		else
		{
			nsNumLen = static_cast<size_t>(std::ceil(std::log10(static_cast<double>(nsPayloadLen) + 1)));
			std::sprintf(reinterpret_cast<char*>(WriteBuffer), "%zu:", nsPayloadLen);
			std::memcpy(WriteBuffer + nsNumLen + 1, nsPayload, nsPayloadLen);
			WriteBuffer[nsNumLen + nsPayloadLen + 1] = ',';
		}

		nsLen = nsNumLen + nsPayloadLen + 2;

		Write(WriteBuffer, nsLen);
		*/
	}

	void UnixStreamSocket::SendBinary(const uint8_t* nsPayload, size_t nsPayloadLen)
	{
std::printf("Entering UnixStreamSocket::SendBinary(const uint8_t* nsPayload, size_t nsPayloadLen).\n");
		//if (this->closed)return;
		//if(this->closed){std::printf("SendBinary() occured, but this-closed is closed.\n");}
/*
		size_t nsNumLen;
		size_t nsLen;

		if (nsPayloadLen > MessageMaxSize)
		{
			MS_ERROR_STD("mesage too big");

			return;
		}

		if (nsPayloadLen == 0)
		{
			nsNumLen       = 1;
			WriteBuffer[0] = '0';
			WriteBuffer[1] = ':';
			WriteBuffer[2] = ',';
		}
		else
		{
			nsNumLen = static_cast<size_t>(std::ceil(std::log10(static_cast<double>(nsPayloadLen) + 1)));
			std::sprintf(reinterpret_cast<char*>(WriteBuffer), "%zu:", nsPayloadLen);
			std::memcpy(WriteBuffer + nsNumLen + 1, nsPayload, nsPayloadLen);
			WriteBuffer[nsNumLen + nsPayloadLen + 1] = ',';
		}

		nsLen = nsNumLen + nsPayloadLen + 2;
		*/

		//Write((const uint8_t*)"papa\0", 5);
	}

void UnixStreamSocket::UserOnUnixStreamRead(void*data)
	{
MS_TRACE_STD();
std::printf("Entering UnixStreamSocket::UserOnUnixStreamRead() %s\n",(char*)data);
//this->listener->mfuck();
// Be ready to parse more than a single message in a single TCP chunk.
std::string text="{\"mama\":\"papa\"}";
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
				std::cout << json << std::endl;
				Channel::Request* request = nullptr;

				try
				{
					std::printf("Creating ::Request.\n");
					request = new Channel::Request(this, json);
				}
				catch (const MediaSoupError& error)
				{
					MS_ERROR_STD("discarding wrong Channel request");
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

			// If there is no more space available in the buffer and that is because
			// the latest parsed message filled it, then empty the full buffer.
			/*
			if ((this->msgStart + readLen) == this->bufferSize)
			{
				this->msgStart      = 0;
				this->bufferDataLen = 0;
			}
			// If there is still space in the buffer, set the beginning of the next
			// parsing to the next position after the parsed message.
			else
			{
				this->msgStart += readLen;
			}
*/
			// If there is more data in the buffer after the parsed message
			// then parse again. Otherwise break here and wait for more data.
			//if (this->bufferDataLen > this->msgStart){continue;}

		//	break;
	//	}
		
		
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
void*set_channel(){
// dummy integer 3 as a parameter, just for fun
auto* chl = new Channel::UnixStreamSocket(3);
return chl;
}
/*
----------------
	without uv_stop()
--------------------
Csignal INT received, exiting
loop::close() occured
room::destroy()
Notifier::Emit(uint32_t targetId, const std::string& event, Json::Value& data) occured.
UnixStreamSocket::Send(Json) occured
{
	"data" : 
	{
		"class" : "Room"
	},
	"event" : "close",
	"targetId" : 35
}
rc fire from_cpp: 0
ON room CLOSED
Room() destructed?
What the fuck in destractor in unixstreamsocket?jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj
on_from_cpp occurred!!! => HALLO from cpp!!!

	------
	here with uv_stop(loop);
	-------
Csignal INT received, exiting
loop::close() occured
room::destroy()
Notifier::Emit(uint32_t targetId, const std::string& event, Json::Value& data) occured.
UnixStreamSocket::Send(Json) occured
{
	"data" : 
	{
		"class" : "Room"
	},
	"event" : "close",
	"targetId" : 35
}
rc fire from_cpp: 0
ON room CLOSED
Room() destructed?
What the fuck in destractor in unixstreamsocket?jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj
libuv loop ended
destroy()
Loop was destroyd?
*/
