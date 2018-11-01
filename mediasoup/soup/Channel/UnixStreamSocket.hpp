#ifndef MS_CHANNEL_UNIX_STREAM_SOCKET_HPP
#define MS_CHANNEL_UNIX_STREAM_SOCKET_HPP
#include "uv_callback.h"
extern uv_callback_t to_cpp;
#ifdef __cplusplus

#include "common.hpp"
#include "Channel/Request.hpp"

#include <json/json.h>


namespace Channel
{
	class UnixStreamSocket
	{
	public:
		class Listener
		{
		public:
			virtual void OnChannelRequest(Channel::UnixStreamSocket* channel, Channel::Request* request) = 0;
			virtual void OnChannelUnixStreamSocketRemotelyClosed(Channel::UnixStreamSocket* channel) = 0;
		virtual void soup_ending() = 0;
		};

	public:
		//explicit 
		UnixStreamSocket(int fd);
		//UnixStreamSocket& operator=(const UnixStreamSocket&)=delete;
		//UnixStreamSocket(const UnixStreamSocket&)=delete;
		

	//private:
	//protected:
		//virtual 
		~UnixStreamSocket();
		//virtual ~Loop();

	public:
		//uv_callback_t to_cpp;
		static void * on_to_cpp(uv_callback_t *callback,void*data);
		static void * close_work(uv_callback_t*callback,void*data);
		static void Soup_Shutdown();
		void SetListener(Listener* listener);
		
		void Send(Json::Value& msg);
		void SendLog(char* nsPayload, size_t nsPayloadLen);
		void SendBinary(const uint8_t* nsPayload, size_t nsPayloadLen);
		
		//by me
		
		//bool IsClosing() const;
		//int if_closing();
		void destroy();

		/* Pure virtual methods inherited from ::UnixStreamSocket. */
	public:
		void UserOnUnixStreamRead(void*data);// override;
		void about_soup_ending();
		void UserOnUnixStreamSocketClosed(bool isClosedByPeer);//override;
		void bfree();
	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		// Others.
		Json::CharReader* jsonReader{ nullptr };
		Json::StreamWriter* jsonWriter{ nullptr };
		size_t msgStart{ 0 }; // Where the latest message starts.
		bool closed{ false };
	};
	
} // namespace Channel

#endif
#ifdef __cplusplus
extern "C"
{
#endif

void * on_from_cpp(uv_callback_t*,void*);
void*set_channel(void);
void soup_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
