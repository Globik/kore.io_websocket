#define MS_CLASS "SignalsHandler"
// #define MS_LOG_DEV

#include "handles/SignalsHandler.hpp"
#include "DepLibUV.hpp"
#include "Logger.hpp"
#include "MediaSoupError.hpp"

/* Static methods for UV callbacks. */

inline static void onSignal(uv_signal_t* handle, int signum)
{
	static_cast<SignalsHandler*>(handle->data)->OnUvSignal(signum);
}

inline static void onClose(uv_handle_t* handle)
{
	delete handle;
}

/* Instance methods. */

SignalsHandler::SignalsHandler(Listener* listener) : listener(listener)
{
	MS_TRACE();
}

void SignalsHandler::AddSignal(int signum, const std::string& name)
{
	MS_TRACE();

	int err;

	auto uvHandle  = new uv_signal_t;
	uvHandle->data = (void*)this;

	err = uv_signal_init(DepLibUV::GetLoop(), uvHandle);
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
	MS_TRACE();

	for (auto uvHandle : uvHandles)
	{
		uv_close(reinterpret_cast<uv_handle_t*>(uvHandle), static_cast<uv_close_cb>(onClose));
	}

	// And delete this.
	delete this;
}

inline void SignalsHandler::OnUvSignal(int signum)
{
	MS_TRACE();

	// Notify the listener.
	this->listener->OnSignal(this, signum);
}
