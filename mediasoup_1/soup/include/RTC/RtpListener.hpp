#ifndef MS_RTC_LISTENER_HPP
#define MS_RTC_LISTENER_HPP

#include "common.hpp"
#include "RTC/RtpPacket.hpp"
#include "RTC/RtpReceiver.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace RTC
{
	class RtpListener
	{
	public:
		Json::Value ToJson() const;
		bool HasSsrc(uint32_t ssrc, const RTC::RtpReceiver* rtpReceiver) const;
		bool HasMuxId(std::string& muxId, const RTC::RtpReceiver* rtpReceiver) const;
		bool HasPayloadType(uint8_t payloadType, const RTC::RtpReceiver* rtpReceiver) const;
		void AddRtpReceiver(RTC::RtpReceiver* rtpReceiver);
		void RemoveRtpReceiver(const RTC::RtpReceiver* rtpReceiver);
		RTC::RtpReceiver* GetRtpReceiver(RTC::RtpPacket* packet);
		RTC::RtpReceiver* GetRtpReceiver(uint32_t ssrc);

	private:
		void RollbackRtpReceiver(
		    RTC::RtpReceiver* rtpReceiver,
		    std::vector<uint32_t>& previousSsrcs,
		    std::string& previousMuxId,
		    std::vector<uint8_t>& previousPayloadTypes);

	public:
		// Table of SSRC / RtpReceiver pairs.
		std::unordered_map<uint32_t, RTC::RtpReceiver*> ssrcTable;
		//  Table of MID RTP header extension / RtpReceiver pairs.
		std::unordered_map<std::string, const RTC::RtpReceiver*> muxIdTable;
		// Table of RTP payload type / RtpReceiver pairs.
		std::unordered_map<uint8_t, RTC::RtpReceiver*> ptTable;
	};

	/* Inline instance methods. */

	inline bool RtpListener::HasSsrc(uint32_t ssrc, const RTC::RtpReceiver* rtpReceiver) const
	{
		auto it = this->ssrcTable.find(ssrc);

		if (it == this->ssrcTable.end())
		{
			return false;
		}
		else
		{
			return (it->second != rtpReceiver);
		}
	}

	inline bool RtpListener::HasMuxId(std::string& muxId, const RTC::RtpReceiver* rtpReceiver) const
	{
		auto it = this->muxIdTable.find(muxId);

		if (it == this->muxIdTable.end())
		{
			return false;
		}
		else
		{
			return (it->second != rtpReceiver);
		}
	}

	inline bool RtpListener::HasPayloadType(uint8_t payloadType, const RTC::RtpReceiver* rtpReceiver) const
	{
		auto it = this->ptTable.find(payloadType);

		if (it == this->ptTable.end())
		{
			return false;
		}
		else
		{
			return (it->second != rtpReceiver);
		}
	}
} // namespace RTC

#endif
