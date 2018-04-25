#define MS_CLASS "RTC::RTCP::FeedbackPsRembPacket"
// #define MS_LOG_DEV

#include "RTC/RTCP/FeedbackPsRemb.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include <cstring>

namespace RTC
{
	namespace RTCP
	{
		/* Class variables. */

		uint32_t FeedbackPsRembPacket::uniqueIdentifier{ 0x52454D42 };

		/* Class methods. */

		FeedbackPsRembPacket* FeedbackPsRembPacket::Parse(const uint8_t* data, size_t len)
		{
			MS_TRACE();

			if (sizeof(CommonHeader) + sizeof(FeedbackPacket::Header) > len)
			{
				MS_WARN_TAG(rtcp, "not enough space for Feedback packet, discarded");

				return nullptr;
			}

			auto* commonHeader = const_cast<CommonHeader*>(reinterpret_cast<const CommonHeader*>(data));

			std::unique_ptr<FeedbackPsRembPacket> packet(new FeedbackPsRembPacket(commonHeader));

			if (!packet->IsCorrect())
				return nullptr;

			return packet.release();
		}

		FeedbackPsRembPacket::FeedbackPsRembPacket(CommonHeader* commonHeader)
		    : FeedbackPsAfbPacket(commonHeader, FeedbackPsAfbPacket::Application::REMB)
		{
			auto* data = reinterpret_cast<uint8_t*>(commonHeader + 1);

			if (Utils::Byte::Get4Bytes(data, 8) != uniqueIdentifier)
			{
				MS_WARN_TAG(rtcp, "invalid unique indentifier in REMB packet");

				this->isCorrect = false;
				return;
			}

			size_t numSsrcs  = data[12];
			uint8_t exponent = data[13] >> 2;
			uint64_t mantissa =
			    (static_cast<uint32_t>(data[13] & 0x03) << 16) | Utils::Byte::Get2Bytes(data, 14);

			this->bitrate = (mantissa << exponent);
			if ((this->bitrate >> exponent) != mantissa)
			{
				MS_WARN_TAG(rtcp, "invalid REMB bitrate value : %" PRIu64 " *2^%u", mantissa, exponent);

				this->isCorrect = false;
				return;
			}

			// Check length.
			size_t len = static_cast<size_t>(ntohs(commonHeader->length) + 1) * 4;

			if (len != sizeof(CommonHeader) + sizeof(FeedbackPacket::Header) + sizeof(Header) +
			               (numSsrcs * sizeof(uint32_t)))
			{
				MS_WARN_TAG(
				    rtcp, "invalid payload size (%zu bytes) for the given number of ssrcs (%zu)", len, numSsrcs);

				this->isCorrect = false;
				return;
			}

			size_t index{ 16 };

			this->ssrcs.reserve(numSsrcs);
			for (size_t n{ 0 }; n < numSsrcs; ++n)
			{
				this->ssrcs.push_back(Utils::Byte::Get4Bytes(data, index));
				index += sizeof(uint32_t);
			}
		}

		size_t FeedbackPsRembPacket::Serialize(uint8_t* buffer)
		{
			MS_TRACE();

			size_t offset     = FeedbackPsPacket::Serialize(buffer);
			uint64_t mantissa = this->bitrate;
			uint8_t exponent{ 0 };

			while (mantissa > 0x3FFFF /* max mantissa (18 bits) */)
			{
				mantissa >>= 1;
				++exponent;
			}

			Utils::Byte::Set4Bytes(buffer, offset, uniqueIdentifier);
			offset += sizeof(uniqueIdentifier);

			buffer[offset] = this->ssrcs.size();
			offset += 1;

			buffer[offset] = (exponent << 2) | (mantissa >> 16);
			offset += 1;

			Utils::Byte::Set2Bytes(buffer, offset, mantissa & 0xFFFF);
			offset += 2;

			for (auto ssrc : this->ssrcs)
			{
				Utils::Byte::Set4Bytes(buffer, offset, ssrc);
				offset += sizeof(ssrc);
			}

			return offset;
		}

		void FeedbackPsRembPacket::Dump() const
		{
			MS_TRACE();

			MS_DUMP("<FeedbackPsRembPacket>");
			FeedbackPsPacket::Dump();
			MS_DUMP("  bitrate (bps): %" PRIu64, this->bitrate);
			for (auto ssrc : this->ssrcs)
			{
				MS_DUMP("  ssrc: %" PRIu32, ssrc);
			}
			MS_DUMP("</FeedbackPsRembPacket>");
		}
	} // namespace RTCP
} // namespace RTC
