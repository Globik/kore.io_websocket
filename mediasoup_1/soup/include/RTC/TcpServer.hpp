#ifndef MS_RTC_TCP_SERVER_HPP
#define MS_RTC_TCP_SERVER_HPP
#ifdef __cplusplus
#include "common.hpp"
#include "RTC/TcpConnection.hpp"
#include "handles/TcpConnection.hpp"
#include "handles/TcpServer.hpp"
#include <uv.h>
#include <unordered_map>

namespace RTC
{
	class TcpServer : public ::TcpServer
	{
	public:
		class Listener
		{
		public:
			virtual ~Listener() = default;

		public:
			virtual void OnRtcTcpConnectionClosed(
			    RTC::TcpServer* tcpServer, RTC::TcpConnection* connection, bool isClosedByPeer) = 0;
		};

	public:
		static void ClassInit();

	private:
		static uv_tcp_t* GetRandomPort(int addressFamily);

	private:
		static struct sockaddr_storage sockaddrStorageIPv4;
		static struct sockaddr_storage sockaddrStorageIPv6;
		static uint16_t minPort;
		static uint16_t maxPort;
		static std::unordered_map<uint16_t, bool> availableIPv4Ports;
		static std::unordered_map<uint16_t, bool> availableIPv6Ports;

	public:
		TcpServer(Listener* listener, RTC::TcpConnection::Listener* connListener, int addressFamily);

	private:
		~TcpServer() override = default;

		/* Pure virtual methods inherited from ::TcpServer. */
	public:
		void UserOnTcpConnectionAlloc(::TcpConnection** connection) override;
		void UserOnNewTcpConnection(::TcpConnection* connection) override;
		void UserOnTcpConnectionClosed(::TcpConnection* connection, bool isClosedByPeer) override;
		void UserOnTcpServerClosed() override;

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		RTC::TcpConnection::Listener* connListener{ nullptr };
	};
} // namespace RTC

#endif

#ifdef __cplusplus
extern "C"
{
#endif
void rtc_tcp_server_class_init(void);
#ifdef __cplusplus
}
#endif
#endif
