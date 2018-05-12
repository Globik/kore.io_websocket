// COM-1 kore-mediasoup
#define MS_CLASS "DepLibUV"
// #define MS_LOG_DEV

#include "DepLibUV.hpp"
#include "Logger.hpp"
#include <cstdlib> // std::abort()

/* Static variables. */

uv_loop_t* DepLibUV::loop{ nullptr };

/* Static methods. */

void DepLibUV::ClassInit()
{
	// NOTE: Logger depends on this so we cannot log anything here.

	int err;

	DepLibUV::loop = new uv_loop_t;
	err            = uv_loop_init(DepLibUV::loop);
	if (err != 0)
		MS_ABORT("libuv initialization failed");
}

void DepLibUV::ClassDestroy()
{
	MS_TRACE();

	// This should never happen.
	if (DepLibUV::loop == nullptr){MS_ABORT("DepLibUV::loop was not allocated");}
std::printf("cCCCCCCCCCCCCCCCCCCCCClosing loop\n");
	//uv_stop(DepLibUV::GetLoop());
	//uv_run(DepLibUV::GetLoop(),UV_RUN_DEFAULT);
	uv_loop_close(DepLibUV::loop);
	delete DepLibUV::loop;
}

void DepLibUV::PrintVersion()
{
	MS_TRACE();

	MS_DEBUG_TAG(info, "loaded libuv version: \"%s\"", uv_version_string());
}

void DepLibUV::RunLoop()
{
	MS_TRACE();

	// This should never happen.
	if (DepLibUV::loop == nullptr)
		MS_ABORT("DepLibUV::loop was not allocated");

	uv_run(DepLibUV::loop, UV_RUN_DEFAULT);
}
// C wrapper
void deplibuv_printversion(){
DepLibUV::PrintVersion();
}
void class_init(){
DepLibUV::ClassInit();
}
void run_loop(){
DepLibUV::RunLoop();
}
void class_destroy(){
DepLibUV::ClassDestroy();
}
uv_loop_t*get_loop(){
return DepLibUV::GetLoop();
}
//uint64_t deplibuv_get_time(){return uint64_t DepLibUV::GetTime();}