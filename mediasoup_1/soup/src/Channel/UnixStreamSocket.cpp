// com-1 kore-mediasoup
#define MS_CLASS "Channel::UnixStreamSocket"
// #define MS_LOG_DEV

#include "DepLibUV.hpp"

#include "Channel/UnixStreamSocket.hpp"
#include "Logger.hpp"
#include "MediaSoupError.hpp"
#include <cmath>   // std::ceil()
#include <cstdio>  // sprintf()
#include <cstring> // std::memmove()
#include <sstream> // std::ostringstream

//by me
#include <string>
#include <unistd.h> //usleep
#include <memory.h>
#include <string.h>
// ebm
/*
extern "C" {
#include <netstring.h>
}
*/

uv_callback_t from_cpp,cb_result;

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
uv_loop_t*mloop=DepLibUV::GetLoop();
uv_loop_set_data(mloop,(void*)this);
		
int rc=uv_callback_init(mloop, &to_cpp, UnixStreamSocket::on_to_cpp, UV_DEFAULT/*COALESCE*/);
std::printf("uv_callback_t &to_cpp init: %d\n",rc);
rc=uv_callback_init(mloop,&from_cpp,on_from_cpp,UV_DEFAULT);
std::printf("uv_callback_t &from_cpp init: %d\n",rc);
		
		rc=uv_callback_init(mloop,&cb_result,on_result,UV_DEFAULT);
std::printf("uv_callback_t &cb_result init: %d\n",rc);
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

	UnixStreamSocket::~UnixStreamSocket()
	{
		MS_TRACE_STD();
int r=uv_callback_fire(&from_cpp,(void*)"exit",NULL);
std::printf("uv_callback_t &from_cpp fire: %d\n",r);
		delete this->jsonReader;
		delete this->jsonWriter;
	}

void * UnixStreamSocket::on_to_cpp(uv_callback_t*callback,void*data)
{
std::printf("uv_callback_t UnixStreamSocket::on_to_cpp occured!: %s\n",(char*)data);
void*bu=uv_loop_get_data(DepLibUV::GetLoop());
static_cast<UnixStreamSocket*>(bu)->UserOnUnixStreamRead((char*)data);
return nullptr;
}
	
	void UnixStreamSocket::SetListener(Listener* listener)
	{
		MS_TRACE_STD();

		this->listener = listener;
	}

	void UnixStreamSocket::Send(Json::Value& msg)
	{
		if (this->closed)
			return;

		// MS_TRACE_STD();

		std::ostringstream stream;
		//std::string nsPayload;
		//size_t nsPayloadLen;
		//size_t nsNumLen;
		//size_t nsLen;

		this->jsonWriter->write(msg, &stream);
		//nsPayload    = stream.str();
		//nsPayloadLen = nsPayload.length();
		const std::string tmp=stream.str();
		//char*wl=strdup(tmp.c_str());

		if (/*nsPayloadLen*/tmp.length() > MessageMaxSize)
		{
			MS_ERROR_STD("mesage too big");

			return;
		}
		char*wl=strdup(tmp.c_str());

		int r=uv_callback_fire(&from_cpp,wl,NULL);
		std::printf("uv_callback_t &from_cpp fire: %d\n",r);

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

		Write(WriteBuffer, nsLen);
		*/
	}

	void UnixStreamSocket::SendLog(char* nsPayload, size_t nsPayloadLen)
	{
		std::printf("sendlog()\n");
		std::printf("Out in: %s :: %d\n", nsPayload,nsPayloadLen);
		
		if (this->closed){std::printf("it looks like this->closed is true\n");return;}
		/*
		char*su=strdup(nsPayload);
		int r=uv_callback_fire(&from_cpp,su, &cb_result);
		std::printf("uv_callback_t &from_cpp fire: %d\n",r);
		*/
/*
		

		size_t nsNumLen;
		size_t nsLen;

		if (nsPayloadLen > MessageMaxSize)
		{
			MS_ERROR_STD("mesage too big");

			return;
		}
		
		//int r=uv_callback_fire(&from_cpp,(void*)nsPayload,NULL);
		//std::printf("uv_callback_t &from_cpp fire: %d\n",r);

		if (nsPayloadLen == 0)
		{
			std::printf("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
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
		//	int r=uv_callback_fire(&from_cpp,(void*)nsPayload, &cb_result);
		//std::printf("uv_callback_t &from_cpp fire: %d\n",r);
		//	usleep(10000);
			//return;

		}

		nsLen = nsNumLen + nsPayloadLen + 2;
//int r=uv_callback_fire(&from_cpp,(void*)WriteBuffer, &cb_result);
	//	std::printf("uv_callback_t &from_cpp fire: %d\n",r);

		//Write(WriteBuffer, nsLen);
		
		*/
	}

	void UnixStreamSocket::SendBinary(const uint8_t* nsPayload, size_t nsPayloadLen)
	{
		std::printf("sendbinary()\n");
		/*
		if (this->closed)
			return;

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

	void UnixStreamSocket::UserOnUnixStreamRead(char*k)
	{
		std::printf("useronunixstreamread()\n");
		MS_TRACE_STD();
std::string text2=k;
		// Be ready to parse more than a single message in a single TCP chunk.
		
		//while (true){
			
			//if (IsClosing())return;
/*
			size_t readLen  = this->bufferDataLen - this->msgStart;
			char* jsonStart = nullptr;
			size_t jsonLen;
			int nsRet = netstring_read(
			    reinterpret_cast<char*>(this->buffer + this->msgStart), readLen, &jsonStart, &jsonLen);

			if (nsRet != 0)
			{
				switch (nsRet)
				{
					case NETSTRING_ERROR_TOO_SHORT:
						// Check if the buffer is full.
						if (this->bufferDataLen == this->bufferSize)
						{
							// First case: the incomplete message does not begin at position 0 of
							// the buffer, so move the incomplete message to the position 0.
							if (this->msgStart != 0)
							{
								std::memmove(this->buffer, this->buffer + this->msgStart, readLen);
								this->msgStart      = 0;
								this->bufferDataLen = readLen;
							}
							// Second case: the incomplete message begins at position 0 of the buffer.
							// The message is too big, so discard it.
							else
							{
								MS_ERROR_STD(
								    "no more space in the buffer for the unfinished message being parsed, "
								    "discarding it");

								this->msgStart      = 0;
								this->bufferDataLen = 0;
							}
						}
						// Otherwise the buffer is not full, just wait.

						// Exit the parsing loop.
						return;

					case NETSTRING_ERROR_TOO_LONG:
						MS_ERROR_STD("NETSTRING_ERROR_TOO_LONG");
						break;

					case NETSTRING_ERROR_NO_COLON:
						MS_ERROR_STD("NETSTRING_ERROR_NO_COLON");
						break;

					case NETSTRING_ERROR_NO_COMMA:
						MS_ERROR_STD("NETSTRING_ERROR_NO_COMMA");
						break;

					case NETSTRING_ERROR_LEADING_ZERO:
						MS_ERROR_STD("NETSTRING_ERROR_LEADING_ZERO");
						break;

					case NETSTRING_ERROR_NO_LENGTH:
						MS_ERROR_STD("NETSTRING_ERROR_NO_LENGTH");
						break;
				}

				// Error, so reset and exit the parsing loop.
				this->msgStart      = 0;
				this->bufferDataLen = 0;

				return;
			}

			// If here it means that jsonStart points to the beginning of a JSON string
			// with jsonLen bytes length, so recalculate readLen.
			readLen = reinterpret_cast<const uint8_t*>(jsonStart) - (this->buffer + this->msgStart) +
			          jsonLen + 1;
*/
			Json::Value json;
			std::string jsonParseError;

			//if (this->jsonReader->parse((const char*)jsonStart, (const char*)jsonStart + jsonLen, &json, &jsonParseError))
			if(this->jsonReader->parse(text2.c_str(),text2.c_str()+text2.size(),&json,&jsonParseError))
			{
				Channel::Request* request = nullptr;

				try
				{
					request = new Channel::Request(this, json);
				}
				catch (const MediaSoupError& error)
				{
					MS_ERROR_STD("discarding wrong Channel request");
				}

				if (request != nullptr)
				{
					// Notify the listener.
					this->listener->OnChannelRequest(this, request);

					// Delete the Request.
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

			// If there is more data in the buffer after the parsed message
			// then parse again. Otherwise break here and wait for more data.
			if (this->bufferDataLen > this->msgStart)
			{
				continue;
			}
*/
		//	break;
		//}
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

void*set_channel(){
// dummy integer 3 as a parameter, just for fun, could be a database instance
auto* chl = new Channel::UnixStreamSocket(3);
return chl;
}
