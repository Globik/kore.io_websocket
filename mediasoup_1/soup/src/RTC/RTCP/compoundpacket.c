#define MS_CLASS "RTC::RTCP::CompoundPacket"
// #define MS_LOG_DEV

// hpp rtc/rtcp/compoundpacket.hpp

#ifndef MS_RTC_RTCP_COMPOUND_PACKET_HPP
#define MS_RTC_RTCP_COMPOUND_PACKET_HPP

#include "common.hpp"
#include "RTC/RTCP/ReceiverReport.hpp"
#include "RTC/RTCP/Sdes.hpp"
#include "RTC/RTCP/SenderReport.hpp"
#include <vector>

namespace RTC
{
	namespace RTCP
	{
		class CompoundPacket
		{
		public:
			const uint8_t* GetData() const;
			size_t GetSize() const;
			size_t GetSenderReportCount() const;
			size_t GetReceiverReportCount() const;
			void Dump() const;
			void AddSenderReport(SenderReport* report);
			void AddReceiverReport(ReceiverReport* report);
			void AddSdesChunk(SdesChunk* chunk);
			void Serialize(uint8_t* data);

		private:
			uint8_t* header{ nullptr };
			size_t size{ 0 };
			SenderReportPacket senderReportPacket;
			ReceiverReportPacket receiverReportPacket;
			SdesPacket sdesPacket;
		};
// by me
struct CompoundPacket{
uint8_t* header{ nullptr };
size_t size{ 0 };
SenderReportPacket senderReportPacket;
ReceiverReportPacket receiverReportPacket;
SdesPacket sdesPacket;
}
		
const uint8_t*CompoundPacket_GetData(struct CompoundPacket*p){
return p->header;
}
		
size_t CompoundPacket_GetSenderReportCount(struct CompoundPacket*p)
{
return p->senderReportPacket.GetCount();
}
size_t CompoundPacket_GetSize(struct CompoundPacket*p){
return p->size;
}
		
		size_t CompoundPacket_GetReceiverReportCount(struct CompoundPacket*p)
		{
			return p->receiverReportPacket.GetCount();
		}
		void CompoundPacket_AddReceiverReport(struct CompoundPacket*p,ReceiverReport* report)
		{
			p->receiverReportPacket.AddReport(report);
		}
		
		void CompoundPacket_AddSdesChunk(struct CompoundPacket*p,SdesChunk* chunk)
		{
			p->sdesPacket.AddChunk(chunk);
		}
//end by me
		/* Inline methods. */

		inline const uint8_t* CompoundPacket::GetData() const
		{
			return this->header;
		}

		inline size_t CompoundPacket::GetSize() const
		{
			return this->size;
		}

		inline size_t CompoundPacket::GetSenderReportCount() const
		{
			return this->senderReportPacket.GetCount();
		}

		inline size_t CompoundPacket::GetReceiverReportCount() const
		{
			return this->receiverReportPacket.GetCount();
		}

		inline void CompoundPacket::AddReceiverReport(ReceiverReport* report)
		{
			this->receiverReportPacket.AddReport(report);
		}

		inline void CompoundPacket::AddSdesChunk(SdesChunk* chunk)
		{
			this->sdesPacket.AddChunk(chunk);
		}
	} // namespace RTCP
} // namespace RTC

#endif


// end of hpp

#include "RTC/RTCP/CompoundPacket.hpp"
#include "Logger.hpp"



namespace RTC
{
	namespace RTCP
	{
		/* Instance methods. */

		void CompoundPacket::Serialize(uint8_t* data)
		{
			MS_TRACE();

			this->header = data;

			// Calculate the total required size for the entire message.
			if (this->senderReportPacket.GetCount() != 0u)
			{
				this->size = this->senderReportPacket.GetSize();

				if (this->receiverReportPacket.GetCount() != 0u)
				{
					this->size += sizeof(ReceiverReport::Header) * this->receiverReportPacket.GetCount();
				}
			}
			// If no sender nor receiver reports are present send an empty Receiver Report
			// packet as the head of the compound packet.
			else
			{
				this->size = this->receiverReportPacket.GetSize();
			}

			if (this->sdesPacket.GetCount() != 0u)
				this->size += this->sdesPacket.GetSize();

			// Fill it.
			size_t offset{ 0 };

			if (this->senderReportPacket.GetCount() != 0u)
			{
				this->senderReportPacket.Serialize(this->header);
				offset = this->senderReportPacket.GetSize();

				// Fix header count field.
				auto* header  = reinterpret_cast<Packet::CommonHeader*>(this->header);
				header->count = 0;

				if (this->receiverReportPacket.GetCount() != 0u)
				{
					// Fix header length field.
					size_t length =
					    ((sizeof(SenderReport::Header) +
					      (sizeof(ReceiverReport::Header) * this->receiverReportPacket.GetCount())) /
					     4);

					header->length = uint16_t{ htons(length) };

					// Fix header count field.
					header->count = this->receiverReportPacket.GetCount();

					auto it = this->receiverReportPacket.Begin();
					for (; it != this->receiverReportPacket.End(); ++it)
					{
						ReceiverReport* report = (*it);

						report->Serialize(this->header + offset);
						offset += sizeof(ReceiverReport::Header);
					}
				}
			}
			else
			{
				this->receiverReportPacket.Serialize(this->header);
				offset = this->receiverReportPacket.GetSize();
			}

			if (this->sdesPacket.GetCount() != 0u)
				this->sdesPacket.Serialize(this->header + offset);
		}

		void CompoundPacket::Dump() const
		{
			MS_TRACE();

			MS_DUMP("<CompoundPacket>");

			if (this->senderReportPacket.GetCount() != 0u)
			{
				this->senderReportPacket.Dump();

				if (this->receiverReportPacket.GetCount() != 0u)
					this->receiverReportPacket.Dump();
			}
			else
				this->receiverReportPacket.Dump();

			if (this->sdesPacket.GetCount() != 0u)
				this->sdesPacket.Dump();

			MS_DUMP("</CompoundPacket>");
		}

		void CompoundPacket::AddSenderReport(SenderReport* report)
		{
			MS_ASSERT(this->senderReportPacket.GetCount() == 0, "a sender report is already present");

			this->senderReportPacket.AddReport(report);
		}
	} // namespace RTCP
} // namespace RTC
