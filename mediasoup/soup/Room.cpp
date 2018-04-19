#define MS_CLASS "RTC::Room"
// #define MS_LOG_DEV

//#include "RTC/Room.hpp"
#include "Room.hpp"
#include "Logger.hpp"
#include "MediaSoupError.hpp"
//#include "Utils.hpp"
#include <cmath> // std::lround()
#include <set>
#include <string>
#include <vector>

namespace RTC
{
	/* Static. */

	static constexpr uint64_t AudioLevelsInterval{ 500 }; // In ms.

	/* Class variables. */

	//RTC::RtpCapabilities Room::supportedRtpCapabilities;

	/* Class methods. */

	void Room::ClassInit()
	{
		MS_TRACE();

		std::printf("Entering Room::ClassInit().\n");
		{
			// NOTE: These lines are auto-generated from data/supportedCapabilities.js.
			const std::string supportedRtpCapabilities =
			    R"({"codecs":[{"kind":"audio","name":"audio/opus","clockRate":48000,"numChannels":2,"rtcpFeedback":[]},{"kind":"audio","name":"audio/PCMU","clockRate":8000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/PCMA","clockRate":8000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/ISAC","clockRate":32000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/ISAC","clockRate":16000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/G722","clockRate":8000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/iLBC","clockRate":8000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/SILK","clockRate":24000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/SILK","clockRate":16000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/SILK","clockRate":12000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/SILK","clockRate":8000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/CN","clockRate":32000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/CN","clockRate":16000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/CN","clockRate":8000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/CN","clockRate":32000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/telephone-event","clockRate":48000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/telephone-event","clockRate":32000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/telephone-event","clockRate":16000,"rtcpFeedback":[]},{"kind":"audio","name":"audio/telephone-event","clockRate":8000,"rtcpFeedback":[]},{"kind":"video","name":"video/VP8","clockRate":90000,"rtcpFeedback":[{"type":"nack"},{"type":"nack","parameter":"pli"},{"type":"nack","parameter":"sli"},{"type":"nack","parameter":"rpsi"},{"type":"nack","parameter":"app"},{"type":"ccm","parameter":"fir"},{"type":"ack","parameter":"rpsi"},{"type":"ack","parameter":"app"},{"type":"goog-remb"}]},{"kind":"video","name":"video/VP9","clockRate":90000,"rtcpFeedback":[{"type":"nack"},{"type":"nack","parameter":"pli"},{"type":"nack","parameter":"sli"},{"type":"nack","parameter":"rpsi"},{"type":"nack","parameter":"app"},{"type":"ccm","parameter":"fir"},{"type":"ack","parameter":"rpsi"},{"type":"ack","parameter":"app"},{"type":"goog-remb"}]},{"kind":"video","name":"video/H264","clockRate":90000,"parameters":{"packetizationMode":0},"rtcpFeedback":[{"type":"nack"},{"type":"nack","parameter":"pli"},{"type":"nack","parameter":"sli"},{"type":"nack","parameter":"rpsi"},{"type":"nack","parameter":"app"},{"type":"ccm","parameter":"fir"},{"type":"ack","parameter":"rpsi"},{"type":"ack","parameter":"app"},{"type":"goog-remb"}]},{"kind":"video","name":"video/H264","clockRate":90000,"parameters":{"packetizationMode":1},"rtcpFeedback":[{"type":"nack"},{"type":"nack","parameter":"pli"},{"type":"nack","parameter":"sli"},{"type":"nack","parameter":"rpsi"},{"type":"nack","parameter":"app"},{"type":"ccm","parameter":"fir"},{"type":"ack","parameter":"rpsi"},{"type":"ack","parameter":"app"},{"type":"goog-remb"}]},{"kind":"video","name":"video/H265","clockRate":90000,"rtcpFeedback":[{"type":"nack"},{"type":"nack","parameter":"pli"},{"type":"nack","parameter":"sli"},{"type":"nack","parameter":"rpsi"},{"type":"nack","parameter":"app"},{"type":"ccm","parameter":"fir"},{"type":"ack","parameter":"rpsi"},{"type":"ack","parameter":"app"},{"type":"goog-remb"}]}],"headerExtensions":[{"kind":"audio","uri":"urn:ietf:params:rtp-hdrext:ssrc-audio-level","preferredId":1,"preferredEncrypt":false},{"kind":"video","uri":"urn:ietf:params:rtp-hdrext:toffset","preferredId":2,"preferredEncrypt":false},{"kind":"","uri":"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time","preferredId":3,"preferredEncrypt":false},{"kind":"video","uri":"urn:3gpp:video-orientation","preferredId":4,"preferredEncrypt":false},{"kind":"","uri":"urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id","preferredId":5,"preferredEncrypt":false}],"fecMechanisms":[]})";

			Json::CharReaderBuilder builder;
			Json::Value settings = Json::nullValue;

			builder.strictMode(&settings);

			Json::CharReader* jsonReader = builder.newCharReader();
			Json::Value json;
			std::string jsonParseError;

			if (!jsonReader->parse(
			        supportedRtpCapabilities.c_str(),
			        supportedRtpCapabilities.c_str() + supportedRtpCapabilities.length(),
			        &json,
			        &jsonParseError))
			{
				delete jsonReader;

				MS_THROW_ERROR_STD(
				    "JSON parsing error in supported RTP capabilities: %s", jsonParseError.c_str());
			}
			else
			{
				delete jsonReader;
			}

			try
			{
			//	Room::supportedRtpCapabilities = RTC::RtpCapabilities(json, RTC::Scope::ROOM_CAPABILITY);
			}
			catch (const MediaSoupError& error)
			{
				MS_THROW_ERROR_STD("wrong supported RTP capabilities: %s", error.what());
			}
		}
	}

	/* Instance methods. */

	Room::Room(Listener* listener, Channel::Notifier* notifier, uint32_t roomId, Json::Value& data)
	    : roomId(roomId), listener(listener), notifier(notifier)
	{
		MS_TRACE();
		std::printf("Entering Room::Room(listener, notifier, roomId, json\n");
		static const Json::StaticString JsonStringMediaCodecs{ "mediaCodecs" };
	}

	Room::~Room()
	{
		std::printf("Look ma, ~Room() destructor!\n");
		MS_TRACE();
	
	}

	void Room::Destroy()
	{
		MS_TRACE();

		static const Json::StaticString JsonStringClass{ "class" };

Json::Value eventData(Json::objectValue);
std::printf("Entering Room::Destroy().\n");
		// Close all the Peers.
		// NOTE: Upon Peer closure the onPeerClosed() method is called which
		// removes it from the map, so this is the safe way to iterate the map
		// and remove elements.
		/*
		for (auto it = this->peers.begin(); it != this->peers.end();)
		{
			auto* peer = it->second;

			it = this->peers.erase(it);
			peer->Destroy();
		}
		*/

		// Close the audio level timer.
		//this->audioLevelsTimer->Destroy();

		// Notify.
		eventData[JsonStringClass] = "Room";
		this->notifier->Emit(this->roomId, "close", eventData);

		// Notify the listener.
		this->listener->OnRoomClosed(this);

		delete this;
	}

	Json::Value Room::ToJson() const
	{
		MS_TRACE();
std::printf("Entering Room::ToJson().\n");
		static const Json::StaticString JsonStringRoomId{ "roomId" };
		static const Json::StaticString JsonStringCapabilities{ "capabilities" };
		static const Json::StaticString JsonStringPeers{ "peers" };
		static const Json::StaticString JsonStringMapRtpReceiverRtpSenders{ "mapRtpReceiverRtpSenders" };
		static const Json::StaticString JsonStringMapRtpSenderRtpReceiver{ "mapRtpSenderRtpReceiver" };
		static const Json::StaticString JsonStringAudioLevelsEventEnabled{ "audioLevelsEventEnabled" };

		Json::Value json(Json::objectValue);
		Json::Value jsonPeers(Json::arrayValue);
		Json::Value jsonMapRtpReceiverRtpSenders(Json::objectValue);
		Json::Value jsonMapRtpSenderRtpReceiver(Json::objectValue);

		// Add `roomId`.
		json[JsonStringRoomId] = Json::UInt{ this->roomId };

		// Add `capabilities`.
		//json[JsonStringCapabilities] = this->capabilities.ToJson();
/*
		// Add `peers`.
		for (auto& kv : this->peers)
		{
			auto* peer = kv.second;

			jsonPeers.append(peer->ToJson());
		}
		json[JsonStringPeers] = jsonPeers;

		// Add `mapRtpReceiverRtpSenders`.
		for (auto& kv : this->mapRtpReceiverRtpSenders)
		{
			auto rtpReceiver = kv.first;
			auto& rtpSenders = kv.second;
			Json::Value jsonRtpReceivers(Json::arrayValue);

			for (auto& rtpSender : rtpSenders)
			{
				jsonRtpReceivers.append(std::to_string(rtpSender->rtpSenderId));
			}

			jsonMapRtpReceiverRtpSenders[std::to_string(rtpReceiver->rtpReceiverId)] = jsonRtpReceivers;
		}
		json[JsonStringMapRtpReceiverRtpSenders] = jsonMapRtpReceiverRtpSenders;

		// Add `mapRtpSenderRtpReceiver`.
		for (auto& kv : this->mapRtpSenderRtpReceiver)
		{
			auto rtpSender   = kv.first;
			auto rtpReceiver = kv.second;

			jsonMapRtpSenderRtpReceiver[std::to_string(rtpSender->rtpSenderId)] =
			    std::to_string(rtpReceiver->rtpReceiverId);
		}
		json[JsonStringMapRtpSenderRtpReceiver] = jsonMapRtpSenderRtpReceiver;

		json[JsonStringAudioLevelsEventEnabled] = this->audioLevelsEventEnabled;
*/
		return json;
	}

	void Room::HandleRequest(Channel::Request* request)
	{
		MS_TRACE();
		std::printf("Entering Room::HandleRequest(request).\n");

		switch (request->methodId)
		{
			case Channel::Request::MethodId::ROOM_CLOSE:
			{
#ifdef MS_LOG_DEV
				uint32_t roomId = this->roomId;
#endif

				Destroy();

				MS_DEBUG_DEV("Room closed [roomId:%" PRIu32 "]", roomId);
std::printf("Room closed [roomId:%" PRIu32 "]", roomId);
				request->Accept();

				break;
			}

			case Channel::Request::MethodId::ROOM_DUMP:
			{
				auto json = ToJson();

				request->Accept(json);

				break;
			}

			case Channel::Request::MethodId::ROOM_CREATE_PEER:
			{
				static const Json::StaticString JsonStringPeerName{ "peerName" };
			request->Accept();

				break;
			}
				
			case Channel::Request::MethodId::ROOM_SET_AUDIO_LEVELS_EVENT:
			{

				request->Accept();

				break;
			}

			
			default:
			{
				MS_ERROR("unknown method");

				request->Reject("unknown method");
			}
		}
	}
} // namespace RTC
// C wrapper
void rtc_room_classinit(){
RTC::Room::ClassInit();
}