#ifndef MS_RTC_RTP_RECEIVER_HPP
#define MS_RTC_RTP_RECEIVER_HPP

#include "common.hpp"
#include "Channel/Notifier.hpp"
#include "Channel/Request.hpp"
#include "RTC/RTCP/CompoundPacket.hpp"
#include "RTC/RTCP/Feedback.hpp"
#include "RTC/RTCP/ReceiverReport.hpp"
#include "RTC/RtpDictionaries.hpp"
#include "RTC/RtpPacket.hpp"
#include "RTC/RtpStreamRecv.hpp"
#include <json/json.h>
#include <map>
#include <string>

namespace RTC
{
	// Avoid cyclic #include problem by declaring classes instead of including
	// the corresponding header files.
	class Transport;

	class RtpReceiver : public RtpStreamRecv::Listener
	{
	public:
		/**
		 * RTC::Peer is the Listener.
		 */
		class Listener
		{
		public:
			virtual void OnRtpReceiverParameters(RTC::RtpReceiver* rtpReceiver)             = 0;
			virtual void OnRtpReceiverParametersDone(RTC::RtpReceiver* rtpReceiver)         = 0;
			virtual void OnRtpPacket(RTC::RtpReceiver* rtpReceiver, RTC::RtpPacket* packet) = 0;
			virtual void OnRtpReceiverClosed(const RTC::RtpReceiver* rtpReceiver)           = 0;
		};

	public:
		RtpReceiver(
		    Listener* listener, Channel::Notifier* notifier, uint32_t rtpReceiverId, RTC::Media::Kind kind);

	private:
		virtual ~RtpReceiver();

	public:
		void Destroy();
		Json::Value ToJson() const;
		void HandleRequest(Channel::Request* request);
		void SetTransport(RTC::Transport* transport);
		RTC::Transport* GetTransport() const;
		void RemoveTransport(RTC::Transport* transport);
		RTC::RtpParameters* GetParameters() const;
		void ReceiveRtpPacket(RTC::RtpPacket* packet);
		void ReceiveRtcpSenderReport(RTC::RTCP::SenderReport* report);
		void GetRtcp(RTC::RTCP::CompoundPacket* packet, uint64_t now);
		void ReceiveRtcpFeedback(RTC::RTCP::FeedbackPsPacket* packet) const;
		void ReceiveRtcpFeedback(RTC::RTCP::FeedbackRtpPacket* packet) const;
		void RequestFullFrame() const;

	private:
		void CreateRtpStream(RTC::RtpEncodingParameters& encoding);
		void ClearRtpStreams();

		/* Pure virtual methods inherited from RTC::RtpStreamRecv::Listener. */
	public:
		void OnNackRequired(RTC::RtpStreamRecv* rtpStream, const std::vector<uint16_t>& seqNumbers) override;
		void OnPliRequired(RTC::RtpStreamRecv* rtpStream) override;

	public:
		// Passed by argument.
		uint32_t rtpReceiverId{ 0 };
		RTC::Media::Kind kind;

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		Channel::Notifier* notifier{ nullptr };
		RTC::Transport* transport{ nullptr };
		// Allocated by this.
		RTC::RtpParameters* rtpParameters{ nullptr };
		std::map<uint32_t, RTC::RtpStreamRecv*> rtpStreams;
		std::map<uint32_t, RTC::RtpStreamRecv*> rtxStreamMap;
		// Others.
		bool rtpRawEventEnabled{ false };
		bool rtpObjectEventEnabled{ false };
		// Timestamp when last RTCP was sent.
		uint64_t lastRtcpSentTime{ 0 };
		uint16_t maxRtcpInterval{ 0 };
	};

	/* Inline methods. */

	inline void RtpReceiver::SetTransport(RTC::Transport* transport)
	{
		this->transport = transport;
	}

	inline RTC::Transport* RtpReceiver::GetTransport() const
	{
		return this->transport;
	}

	inline void RtpReceiver::RemoveTransport(RTC::Transport* transport)
	{
		if (this->transport == transport)
			this->transport = nullptr;
	}

	inline RTC::RtpParameters* RtpReceiver::GetParameters() const
	{
		return this->rtpParameters;
	}

	inline void RtpReceiver::ReceiveRtcpSenderReport(RTC::RTCP::SenderReport* report)
	{
		auto it = this->rtpStreams.find(report->GetSsrc());
		if (it != this->rtpStreams.end())
		{
			auto rtpStream = it->second;

			rtpStream->ReceiveRtcpSenderReport(report);
		}
	}
} // namespace RTC

#endif
