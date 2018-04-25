#define MS_CLASS "RTC::RtpStreamRecv"
// #define MS_LOG_DEV

#include "RTC/RtpStreamRecv.hpp"
#include "DepLibUV.hpp"
#include "Logger.hpp"

namespace RTC
{
	/* Instance methods. */

	RtpStreamRecv::RtpStreamRecv(Listener* listener, RTC::RtpStream::Params& params)
	    : RtpStream::RtpStream(params), listener(listener)
	{
		MS_TRACE();
	}

	RtpStreamRecv::~RtpStreamRecv()
	{
		MS_TRACE();
	}

	Json::Value RtpStreamRecv::ToJson() const
	{
		MS_TRACE();

		static const Json::StaticString JsonStringParams{ "params" };
		static const Json::StaticString JsonStringReceived{ "received" };
		static const Json::StaticString JsonStringMaxTimestamp{ "maxTimestamp" };
		static const Json::StaticString JsonStringTransit{ "transit" };
		static const Json::StaticString JsonStringJitter{ "jitter" };

		Json::Value json(Json::objectValue);

		json[JsonStringParams]       = this->params.ToJson();
		json[JsonStringReceived]     = Json::UInt{ this->received };
		json[JsonStringMaxTimestamp] = Json::UInt{ this->maxTimestamp };
		json[JsonStringTransit]      = Json::UInt{ this->transit };
		json[JsonStringJitter]       = Json::UInt{ this->jitter };

		return json;
	}

	bool RtpStreamRecv::ReceivePacket(RTC::RtpPacket* packet)
	{
		MS_TRACE();

		// Call the parent method.
		if (!RtpStream::ReceivePacket(packet))
			return false;

		// Calculate Jitter.
		CalculateJitter(packet->GetTimestamp());

		// Set RTP header extension ids.
		this->SetHeaderExtensions(packet);

		// Pass the packet to the NackGenerator.
		if (this->params.useNack)
			this->nackGenerator->ReceivePacket(packet);

		return true;
	}

	bool RtpStreamRecv::ReceiveRtxPacket(RTC::RtpPacket* packet)
	{
		MS_TRACE();

		MS_ASSERT(packet->GetSsrc() == this->rtxSsrc, "invalid ssrc on rtx packet");

		// Check that the payload type corresponds to the one negotiated.
		if (packet->GetPayloadType() != this->rtxPayloadType)
		{
			MS_WARN_TAG(
			    rtx,
			    "ignoring rtx packet with invalid payload type [ssrc: %" PRIu32 " seqnr: %" PRIu16
			    " payload type: %" PRIu8 "]",
			    packet->GetSsrc(),
			    packet->GetSequenceNumber(),
			    packet->GetPayloadType());

			return false;
		}

		// Get the rtx packet sequence number for logging purposes.
		auto rtxSeq = packet->GetSequenceNumber();

		// Get the original rtp packet.
		if (!packet->RtxDecode(this->params.payloadType, this->params.ssrc))
		{
			MS_WARN_TAG(
			    rtx,
			    "ignoring malformed rtx packet [ssrc: %" PRIu32 " seqnr: %" PRIu16
			    " payload type: %" PRIu8 "]",
			    packet->GetSsrc(),
			    packet->GetSequenceNumber(),
			    packet->GetPayloadType());

			return false;
		}

		MS_DEBUG_TAG(
		    rtx,
		    "received rtx packet [ssrc: %" PRIu32 " seqnr: %" PRIu16
		    "] recovering original [ssrc: %" PRIu32 " seqnr: %" PRIu16 "]",
		    this->rtxSsrc,
		    rtxSeq,
		    packet->GetSsrc(),
		    packet->GetSequenceNumber());

		// Set the extended sequence number into the packet.
		packet->SetExtendedSequenceNumber(
		    this->cycles + static_cast<uint32_t>(packet->GetSequenceNumber()));

		// Set RTP header extension ids.
		this->SetHeaderExtensions(packet);

		// Pass the packet to the NackGenerator.
		if (this->params.useNack && this->nackGenerator)
			this->nackGenerator->ReceivePacket(packet);

		return true;
	}

	RTC::RTCP::ReceiverReport* RtpStreamRecv::GetRtcpReceiverReport()
	{
		MS_TRACE();

		auto report = new RTC::RTCP::ReceiverReport();

		// Calculate Packets Expected and Lost.
		uint32_t expected = (this->cycles + this->maxSeq) - this->baseSeq + 1;
		int32_t totalLost = expected - this->received;

		report->SetTotalLost(totalLost);

		// Calculate Fraction Lost.
		uint32_t expectedInterval = expected - this->expectedPrior;

		this->expectedPrior = expected;

		uint32_t receivedInterval = this->received - this->receivedPrior;

		this->receivedPrior = this->received;

		int32_t lostInterval = expectedInterval - receivedInterval;
		uint8_t fractionLost;

		if (expectedInterval == 0 || lostInterval <= 0)
			fractionLost = 0;
		else
			fractionLost = (lostInterval << 8) / expectedInterval;

		report->SetFractionLost(fractionLost);

		// Fill the rest of the report.
		report->SetLastSeq(static_cast<uint32_t>(this->maxSeq) + this->cycles);
		report->SetJitter(this->jitter);

		if (this->lastSrReceived != 0u)
		{
			// Get delay in milliseconds.
			auto delayMs = static_cast<uint32_t>(DepLibUV::GetTime() - this->lastSrReceived);
			// Express delay in units of 1/65536 seconds.
			uint32_t dlsr = (delayMs / 1000) << 16;

			dlsr |= uint32_t{ (delayMs % 1000) * 65536 / 1000 };
			report->SetDelaySinceLastSenderReport(dlsr);
			report->SetLastSenderReport(this->lastSrTimestamp);
		}
		else
		{
			report->SetDelaySinceLastSenderReport(0);
			report->SetLastSenderReport(0);
		}

		return report;
	}

	void RtpStreamRecv::ReceiveRtcpSenderReport(RTC::RTCP::SenderReport* report)
	{
		MS_TRACE();

		this->lastSrReceived  = DepLibUV::GetTime();
		this->lastSrTimestamp = report->GetNtpSec() << 16;
		this->lastSrTimestamp += report->GetNtpFrac() >> 16;
	}

	void RtpStreamRecv::RequestFullFrame()
	{
		MS_TRACE();

		if (this->params.usePli)
		{
			// Reset NackGenerator.
			if (this->params.useNack)
				this->nackGenerator.reset(new RTC::NackGenerator(this));

			this->listener->OnPliRequired(this);
		}
	}

	void RtpStreamRecv::CalculateJitter(uint32_t rtpTimestamp)
	{
		MS_TRACE();

		if (this->params.clockRate == 0u)
			return;

		auto transit =
		    static_cast<int>(DepLibUV::GetTime() - (rtpTimestamp * 1000 / this->params.clockRate));
		int d = transit - this->transit;

		this->transit = transit;
		if (d < 0)
			d = -d;
		this->jitter += (1. / 16.) * (static_cast<double>(d) - this->jitter);
	}

	void RtpStreamRecv::OnInitSeq()
	{
		MS_TRACE();

		// Reset NackGenerator.
		if (this->params.useNack)
			this->nackGenerator.reset(new RTC::NackGenerator(this));

		// Request a full frame so dropped video packets don't cause lag.
		if (this->params.mime.type == RTC::RtpCodecMime::Type::VIDEO)
		{
			MS_DEBUG_TAG(rtx, "stream initialized, triggering PLI [ssrc:%" PRIu32 "]", this->params.ssrc);

			this->listener->OnPliRequired(this);
		}
	}

	void RtpStreamRecv::OnNackRequired(const std::vector<uint16_t>& seqNumbers)
	{
		MS_TRACE();

		MS_ASSERT(this->params.useNack, "NACK required but not supported");

		MS_WARN_TAG(
		    rtx,
		    "triggering NACK [ssrc:%" PRIu32 ", first seq:%" PRIu16 ", num packets:%zu]",
		    this->params.ssrc,
		    seqNumbers[0],
		    seqNumbers.size());

		this->listener->OnNackRequired(this, seqNumbers);
	}

	void RtpStreamRecv::OnFullFrameRequired()
	{
		MS_TRACE();

		if (!this->params.usePli)
		{
			MS_WARN_TAG(rtx, "PLI required but not supported by the endpoint");

			return;
		}

		MS_DEBUG_TAG(rtx, "triggering PLI [ssrc:%" PRIu32 "]", this->params.ssrc);

		this->listener->OnPliRequired(this);
	}

	void RtpStreamRecv::SetRtx(uint8_t payloadType, uint32_t ssrc)
	{
		MS_TRACE();

		this->hasRtx         = true;
		this->rtxPayloadType = payloadType;
		this->rtxSsrc        = ssrc;
	}

	void RtpStreamRecv::SetHeaderExtensions(RTC::RtpPacket* packet) const
	{
		MS_TRACE();

		if (this->params.ssrcAudioLevelId != 0u)
		{
			packet->AddExtensionMapping(
			    RtpHeaderExtensionUri::Type::SSRC_AUDIO_LEVEL, this->params.ssrcAudioLevelId);
		}

		if (this->params.absSendTimeId != 0u)
		{
			packet->AddExtensionMapping(
			    RtpHeaderExtensionUri::Type::ABS_SEND_TIME, this->params.absSendTimeId);
		}
	}
} // namespace RTC
