// /home/globik/kore-mediasoup/training/luvi/l/
#ifndef LIBUV_HPP
#define LIBUV_HPP
#include <uv.h>
#include <unistd.h>
#include "uv_callback.h"
#ifdef __cplusplus
#include <iostream>
//#include <functional>
#include <string>
class deplibuv
{
public:
	static void classinit();
	static void classdestroy();
	static void printversion();
	static void runloop();
	static uv_loop_t* getloop();
	static uint64_t gettime();
	
	//int hello(const std::string& s);
	/*{
	std::cout << "hello " << s << '\n';
	}
	*/
	
	static void * on_to_cpp(uv_callback_t *callback,void*data);
	
	//void display(char*text);//{std::cout << text << std::endl;};ddd
	
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
void*trans_to_cpp(uv_callback_t*,void*);
void * on_from_cpp(uv_callback_t *,void*);
void printi(void);
void ini(void);
void destri(void);
void runi(void);
uv_loop_t*get_loopi(void);
#ifdef __cplusplus
}
#endif
#endif