#define MS_CLASS "Loop"
// #define MS_LOG_DEV

#include "Loop.hpp"
#include "deplibuv.hpp"
#include "Logger.hpp"
#include "MediaSoupError.hpp"
#include "Settings.hpp"
#include <json/json.h>
#include <cerrno>
#include <iostream> // std::cout, std::cerr
#include <string>
#include <utility> // std::pair()
#include <unistd.h>//usleep
/* Instance methods. */

Loop::Loop(Channel::UnixStreamSocket* channel) : channel(channel)
{
	//MS_TRACE();

	std::printf("Set us as Channel's listener.\n");
	this->channel->SetListener(this);
	//this->channel->listener->mfuck();
	Loop::mfuck();

	// Create the Notifier instance.
	this->notifier = new Channel::Notifier(this->channel);

	// Set the signals handler.
	this->signalsHandler = new SignalsHandler(this);

	// Add signals to handle.
	this->signalsHandler->AddSignal(SIGINT, "INT");
	this->signalsHandler->AddSignal(SIGTERM, "TERM");

	std::printf("starting libuv loop\n");
	deplibuv::runloop();
	//uv_stop(deplibuv::getloop());
std::printf("libuv loop ended\n");
}

Loop::~Loop()
{
	std::printf("loop destructer occured llllllllllllllllllllllllloooop destructure\n");
	MS_TRACE();
}
//C wrapper

void Loop::Close()
{
	MS_TRACE();
std::printf("loop::close() occured\n");
	if (this->closed)
	{
		MS_ERROR("already closed");

		return;
	}

	this->closed = true;

	// Close the SignalsHandler.
	if (this->signalsHandler != nullptr){
		std::printf("CLOSE SIGNALSHANDLER DESTROY\n");
		this->signalsHandler->Destroy();
	}

	// Close all the Rooms.
	// NOTE: Upon Room closure the onRoomClosed() method is called which
	// removes it from the map, so this is the safe way to iterate the map
	// and remove elements.
	for (auto it = this->rooms.begin(); it != this->rooms.end();)
	{
		RTC::Room* room = it->second;

		it = this->rooms.erase(it);
		room->Destroy();
	}
	
//usleep(1000000);
//uv_stop(deplibuv::getloop());

	// Delete the Notifier.
	this->channel->~UnixStreamSocket();
	delete this->notifier;
//this->channel->~UnixStreamSocket();
	//delete this->channel;
	// Close the Channel socket.
	//if (this->channel != nullptr)this->channel->Destroy();
	//usleep(100000);
	//uv_stop(deplibuv::getloop());
	//this->~Loop();
	//Loop::~Loop(this->channel);
}

RTC::Room* Loop::GetRoomFromRequest(Channel::Request* request, uint32_t* roomId)
{
	//MS_TRACE();
std::printf("get room from request\n");
	static const Json::StaticString JsonStringRoomId{ "roomId" };

	auto jsonRoomId = request->internal[JsonStringRoomId];

	if (!jsonRoomId.isUInt())
		MS_THROW_ERROR("Request has not numeric internal.roomId");

	// If given, fill roomId.
	if (roomId != nullptr)
		*roomId = jsonRoomId.asUInt();

	auto it = this->rooms.find(jsonRoomId.asUInt());
	if (it != this->rooms.end())
	{
		RTC::Room* room = it->second;
std::printf("some room found, giving it pass.\n");
		return room;
	}
std::printf("No room found.\n");
	return nullptr;
}

void Loop::OnSignal(SignalsHandler* /*signalsHandler*/, int signum)
{
	MS_TRACE();

	switch (signum)
	{
		case SIGINT:
			std::printf("signal INT received, exiting\n");
			Close();
			break;

		case SIGTERM:
			std::printf("signal TERM received, exiting\n");
			Close();
			break;

		default:
			std::printf("received a signal (with signum %d) for which there is no handling code\n", signum);
	}
}
void Loop::mfuck(){
std::printf("loop::mfuck\n");
}

void Loop::OnChannelRequest(Channel::UnixStreamSocket* channel, Channel::Request* request)
{
	std::printf("Loop::OnChannelRequest()\n");
	MS_TRACE();

	//MS_DEBUG_DEV("'%s' request", request->method.c_str());
//std::printf("Loop::OnChannelRequest()\n");
//	return;
	switch (request->methodId)
	{
		case Channel::Request::MethodId::WORKER_DUMP:
		{
			static const Json::StaticString JsonStringWorkerId{ "workerId" };
			static const Json::StaticString JsonStringRooms{ "rooms" };

			Json::Value json(Json::objectValue);
			Json::Value jsonRooms(Json::arrayValue);

			json[JsonStringWorkerId] = Logger::id;

			for (auto& kv : this->rooms)
			{
				auto room = kv.second;

				jsonRooms.append(room->ToJson());
			}

			json[JsonStringRooms] = jsonRooms;

			request->Accept(json);

			break;
		}

		case Channel::Request::MethodId::WORKER_UPDATE_SETTINGS:
		{
			//Settings::HandleRequest(request);

			break;
		}

		case Channel::Request::MethodId::WORKER_CREATE_ROOM:
		{
			static const Json::StaticString JsonStringCapabilities{ "capabilities" };

			RTC::Room* room;
			uint32_t roomId;

			try
			{
				room = GetRoomFromRequest(request, &roomId);
			}
			catch (const MediaSoupError& error)
			{
				request->Reject(error.what());

				return;
			}

			if (room != nullptr)
			{
				request->Reject("Room already exists");

				return;
			}

			try
			{
				std::printf("The room must be created.\n");
				room = new RTC::Room(this, this->notifier, roomId, request->data);
			}
			catch (const MediaSoupError& error)
			{
				std::printf("Fail to create the room.\n");
				request->Reject(error.what());

				return;
			}

			this->rooms[roomId] = room;

			MS_DEBUG_DEV("Room created [roomId:%" PRIu32 "]", roomId);
std::printf("Room created roomId:%" PRIu32 "]\n",roomId);
			Json::Value data(Json::objectValue);

			// Add `capabilities`.
			//data[JsonStringCapabilities] = room->GetCapabilities().ToJson();

			request->Accept(data);

			break;
		}

		case Channel::Request::MethodId::ROOM_CLOSE:
		case Channel::Request::MethodId::ROOM_DUMP:
		case Channel::Request::MethodId::ROOM_CREATE_PEER:
		case Channel::Request::MethodId::ROOM_SET_AUDIO_LEVELS_EVENT:
		case Channel::Request::MethodId::PEER_CLOSE:
		case Channel::Request::MethodId::PEER_DUMP:
		case Channel::Request::MethodId::PEER_SET_CAPABILITIES:
		case Channel::Request::MethodId::PEER_CREATE_TRANSPORT:
		case Channel::Request::MethodId::PEER_CREATE_RTP_RECEIVER:
		case Channel::Request::MethodId::TRANSPORT_CLOSE:
		case Channel::Request::MethodId::TRANSPORT_DUMP:
		case Channel::Request::MethodId::TRANSPORT_SET_REMOTE_DTLS_PARAMETERS:
		case Channel::Request::MethodId::TRANSPORT_SET_MAX_BITRATE:
		case Channel::Request::MethodId::TRANSPORT_CHANGE_UFRAG_PWD:
		case Channel::Request::MethodId::RTP_RECEIVER_CLOSE:
		case Channel::Request::MethodId::RTP_RECEIVER_DUMP:
		case Channel::Request::MethodId::RTP_RECEIVER_RECEIVE:
		case Channel::Request::MethodId::RTP_RECEIVER_SET_TRANSPORT:
		case Channel::Request::MethodId::RTP_RECEIVER_SET_RTP_RAW_EVENT:
		case Channel::Request::MethodId::RTP_RECEIVER_SET_RTP_OBJECT_EVENT:
		case Channel::Request::MethodId::RTP_SENDER_DUMP:
		case Channel::Request::MethodId::RTP_SENDER_SET_TRANSPORT:
		case Channel::Request::MethodId::RTP_SENDER_DISABLE:
		{
			RTC::Room* room;

			try
			{
				room = GetRoomFromRequest(request);
			}
			catch (const MediaSoupError& error)
			{
				request->Reject(error.what());

				return;
			}

			if (room == nullptr)
			{
				request->Reject("Room does not exist");

				return;
			}

			room->HandleRequest(request);

			break;
		}

		default:
		{
			MS_ERROR("unknown method");

			request->Reject("unknown method");
		}
	}
}

void Loop::OnChannelUnixStreamSocketRemotelyClosed(Channel::UnixStreamSocket* /*socket*/)
{
	MS_TRACE_STD();

	std::printf("When mediasoup Node process ends it sends a SIGTERM to us so we close this\n");
	// pipe and then exit.
	// If the pipe is remotely closed it means that mediasoup Node process
	// abruptly died (SIGKILL?) so we must die.
	MS_ERROR_STD("Channel remotely closed, killing myself");

	this->channel = nullptr;
	Close();
}

void Loop::OnRoomClosed(RTC::Room* room)
{
	MS_TRACE();
std::printf("ON room CLOSED\n");
	this->rooms.erase(room->roomId);
}

void suka(void*fi){
Loop loop(static_cast<Channel::UnixStreamSocket*>(fi));
std::printf("SUUUUUUUUUUUUUUUUUUUKA!\n");
}
