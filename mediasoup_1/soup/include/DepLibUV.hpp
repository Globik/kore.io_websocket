// COM-1 kore-mediasoup
#ifndef MS_DEP_LIBUV_HPP
#define MS_DEP_LIBUV_HPP


#include <uv.h>
#ifdef __cplusplus
#include "common.hpp"
#include <iostream>

class DepLibUV
{
public:
	static void ClassInit();
	static void ClassDestroy();
	static void PrintVersion();
	static void RunLoop();
	static uv_loop_t* GetLoop();
	static uint64_t GetTime();

private:
	static uv_loop_t* loop;
};

/* Inline static methods. */

inline uv_loop_t* DepLibUV::GetLoop()
{
	return DepLibUV::loop;
}

inline uint64_t DepLibUV::GetTime()
{
	return uv_now(DepLibUV::loop);
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif
void deplibuv_printversion(void);
void class_init(void);
void run_loop(void);
void class_destroy(void);
uv_loop_t*get_loop(void);
uint64_t deplibuv_get_time(void);
#ifdef __cplusplus
}
#endif
#endif
