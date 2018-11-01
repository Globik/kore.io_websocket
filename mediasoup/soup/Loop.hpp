//#ifndef MS_LOOP_HPP
//#define MS_LOOP_HPP

#ifdef __cplusplus
#include "common.hpp"
#include "Channel/Notifier.hpp"
#include "Channel/Request.hpp"
#include "Channel/UnixStreamSocket.hpp"

#include "Room.hpp"
#include "handles/SignalsHandler.hpp"
#include <unordered_map>
// from kore.io_websocket/mediasoup
class Loop : public SignalsHandler::Listener,
             public Channel::UnixStreamSocket::Listener,
             public RTC::Room::Listener
{
public:
	explicit Loop(Channel::UnixStreamSocket* channel);
	//explicit Loop();
	~Loop();// override;
	//Loop get_loop();
	void dclose();
	//void Close();
	

private:
	void Close();
	RTC::Room* GetRoomFromRequest(Channel::Request* request, uint32_t* roomId = nullptr);

	/* Methods inherited from SignalsHandler::Listener. */
public:
	void OnSignal(SignalsHandler* signalsHandler, int signum) override;
	

	/* Methods inherited from Channel::lUnixStreamSocket::Listener. */
public:
	void OnChannelRequest(Channel::UnixStreamSocket* channel, Channel::Request* request) override;
	void OnChannelUnixStreamSocketRemotelyClosed(Channel::UnixStreamSocket* channel) override;
	void mfuck() override;

	/* Methods inherited from RTC::Room::Listener. */
public:
	void OnRoomClosed(RTC::Room* room) override;

private:
	// Passed by argument.
	Channel::UnixStreamSocket* channel{ nullptr };
	// Allocated by this.
	Channel::Notifier* notifier{ nullptr };
	SignalsHandler* signalsHandler{ nullptr };
	// Others.
	bool closed{ false };
	//void Bose();
	std::unordered_map<uint32_t, RTC::Room*> rooms;
	Loop*faka{nullptr};
};
//inline void dclose(){Loop*l=this;l->Close();}
#endif

	


#ifdef __cplusplus
extern "C"
{
#endif

void suka(void*);
void Close_soup(void);
	
#ifdef __cplusplus
}
#endif

