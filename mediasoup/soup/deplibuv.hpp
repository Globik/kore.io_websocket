#ifndef LIBUV_HPP
#define LIBUV_HPP
//#pragma once
//#include "fi.hpp"
#include <uv.h>

#ifdef __cplusplus
#include <iostream>

#include <string>

class deplibuv
{
public:
	static int classinit();
	static int classdestroy();
	static void printversion();
	static int runloop();
	static uv_loop_t* getloop();
	static uint64_t gettime();
	void display(char*text);
private:
	static uv_loop_t*loop;
};
inline uv_loop_t* deplibuv::getloop(){
return deplibuv::loop;
}
inline uint64_t deplibuv::gettime(){
return uv_now(deplibuv::loop);
}

#endif

#ifdef __cplusplus
extern "C"
{
#endif
void deplibuv_printversion(void);
//int class_init(void);
int class_init(void);
//void run_loop(void);
void class_destroy(void);
uv_loop_t*get_loopi(void);
#ifdef __cplusplus
}
#endif



#endif
