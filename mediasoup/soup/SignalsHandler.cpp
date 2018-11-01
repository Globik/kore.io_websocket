#define MS_CLASS "SignalsHandler"
// #define MS_LOG_DEV

#include "handles/SignalsHandler.hpp"
#include "deplibuv.hpp"
#include "Logger.hpp"
#include "MediaSoupError.hpp"

/* Static methods for UV callbacks. */

// to hell with sigint. wegen sigint the main process of kore.c no more works and wan't to shutdown gracefully
// so I decided to remove signalshandler from a subthread 

inline static void onSignal(uv_signal_t* handle, int signum)
{//unused
	static_cast<SignalsHandler*>(handle->data)->OnUvSignal(signum);
}

inline static void onClose(uv_handle_t* handle)
{//unused
	printf("on close sig handler\n");
	delete handle;
}

/* Instance methods. */

SignalsHandler::SignalsHandler(Listener* listener) : listener(listener)
{
	MS_TRACE();
}

void SignalsHandler::AddSignal(int signum, const std::string& name)
{
	//unused
	MS_TRACE();
printf("adding signal\n");
	int err;

	auto uvHandle  = new uv_signal_t;
	uvHandle->data = (void*)this;

	err = uv_signal_init(deplibuv::getloop(), uvHandle);
	if (err != 0)
	{
		delete uvHandle;

		MS_THROW_ERROR("uv_signal_init() failed for signal %s: %s", name.c_str(), uv_strerror(err));
	}

	err = uv_signal_start(uvHandle, static_cast<uv_signal_cb>(onSignal), signum);
	if (err != 0)
		MS_THROW_ERROR("uv_signal_start() failed for signal %s: %s", name.c_str(), uv_strerror(err));

	// Enter the UV handle into the vector.
	this->uvHandles.push_back(uvHandle);
}

void SignalsHandler::Destroy()
{
	//unused
	MS_TRACE();
printf("signal destroy\n");
//by me => a fake signal 4 :)
this->listener->OnSignal(this, 1);
	for (auto uvHandle : uvHandles)
	{
		//unused
		uv_close(reinterpret_cast<uv_handle_t*>(uvHandle), static_cast<uv_close_cb>(onClose));
	}

	printf("And delete this signal\n");
	delete this;//??? wegen das this->closed is undefined in a function SendLog, in uv_callback, at shutdown, last data is not freed.
}

inline void SignalsHandler::OnUvSignal(int signum)
{
	// by me unused
	MS_TRACE();

	// Notify the listener.
	this->listener->OnSignal(this, signum);
}
