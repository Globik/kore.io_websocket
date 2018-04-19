#ifndef MS_RTC_ROOM_HPP
#define MS_RTC_ROOM_HPP
#ifdef __cplusplus

#include "common.hpp"
#include "Channel/Notifier.hpp"
#include "Channel/Request.hpp"
/*
#include "RTC/Peer.hpp"
#include "RTC/RTCP/Feedback.hpp"
#include "RTC/RTCP/ReceiverReport.hpp"
#include "RTC/RTCP/Sdes.hpp"
#include "RTC/RTCP/SenderReport.hpp"
#include "RTC/RtpDictionaries.hpp"
#include "RTC/RtpPacket.hpp"
#include "RTC/RtpReceiver.hpp"
#include "RTC/RtpSender.hpp"
#include "handles/Timer.hpp"
*/
#include <json/json.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace RTC
{
	class Room /*: public RTC::Peer::Listener, public Timer::Listener*/
	{
	public:
		class Listener
		{
		public:
			virtual void OnRoomClosed(RTC::Room* room) = 0;
		};

	public:
		static void ClassInit();

	private:
	//	static RTC::RtpCapabilities supportedRtpCapabilities;
	//	static std::vector<uint8_t> availablePayloadTypes;

	public:
		Room(Listener* listener, Channel::Notifier* notifier, uint32_t roomId, Json::Value& data);

	private:
		virtual ~Room();

	public:
		void Destroy();
		Json::Value ToJson() const;
		void HandleRequest(Channel::Request* request);
	//	const RTC::RtpCapabilities& GetCapabilities() const;

	private:
	 

		/* Pure virtual methods inherited from Timer::Listener. */
	//public:
	//	void OnTimer(Timer* timer) override;

	public:
		// Passed by argument.
		uint32_t roomId{ 0 };

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		Channel::Notifier* notifier{ nullptr };
	
	};

	/* Inline static methods. */


} // namespace RTC

#endif

#ifdef __cplusplus
extern "C"
{
	#endif
	void rtc_room_classinit(void);
	#ifdef __cplusplus
}
#endif

#endif
