// kore.io soup
//#define MS_CLASS "deplibuv"
#include <unistd.h>
//#include "fi.hpp"
#include "deplibuv.hpp"

//#include "Channel/UnixStreamSocket.hpp"
//#include "Logger.hpp"

//#include "MediaSoupError.hpp"


#include <iostream>
#include <cstdlib>

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
uv_loop_t* deplibuv::loop{nullptr};
//uv_callback_t from_cpp;

int deplibuv::classinit(){
std::cout << "Entering deplibuv::classinit()\n";

int err;
	deplibuv::loop=new uv_loop_t;
	err=uv_loop_init(deplibuv::loop);
	if(err !=0){
	std::cout << red "Libuv init failed.\n" rst;
	return 1;
	//MS_ABORT("libuv initialization failed");
	}
	return 0;
}
// just another dummy method while developing
void deplibuv::display(char*text){std::cout << text << std::endl;};

void on_clo_unx3(uv_handle_t*client){
printf("On_clo3() occurred.\n");
}

void on_walk3(uv_handle_t*handle, void * arg){
printf("on_walk unixstream socket3\n");
uv_close(handle, on_clo_unx3);
}


int deplibuv::classdestroy(){
printf("deplibuv::classdesproy\n");
if(deplibuv::loop==nullptr){ 
std::cout << red "Loop was not allocated!\n" rst;
return 1;
}

int a=uv_loop_close(deplibuv::loop);
printf(red "a %d\n" rst, a);
if(a<0){
//int r=uv_callback_fire(&from_cpp, NULL,NULL);
//std::printf(yellow "uv_callback_t &from_cpp fire at SHUTDOWN LEVEL: %d\n" rst,r);
uv_walk(deplibuv::getloop(), on_walk3, NULL);
}
delete deplibuv::loop;
deplibuv::loop=nullptr;

std::cout << yellow "Look ma, loop is destroyd." rst << std::endl;
return 0;
}


void deplibuv::printversion(){
std::cout <<  green "Libuv version:  " rst << uv_version_string() << std::endl;
}

int deplibuv::runloop(){
if(deplibuv::loop==nullptr){
std::cout << red "loop was not allocated 2!\n" rst << std::endl;
//std::abort();
return 1;
}
	
std::cout << green "Loop is allocated successfully" rst << std::endl;
uv_run(deplibuv::loop,UV_RUN_DEFAULT);
return 0;
}

// C wrapper
void deplibuv_printversion(){
deplibuv::printversion();
}
static int class_v_init(){
	/*
try{
deplibuv::classinit();
return 0;
}catch(const MediaSoupError &error){
	printf(red " *** ERRRRORRR! in class_init() %s\n" rst, error.what());
MS_ERROR("Conf error: %s", error.what());
return 1;	
}
return 0;
*/ 
}
int class_init(){
int as=deplibuv::classinit();
return as;
}

void class_destroy(){
deplibuv::classdestroy();
}
uv_loop_t*get_loopi(){
return deplibuv::getloop();
}
