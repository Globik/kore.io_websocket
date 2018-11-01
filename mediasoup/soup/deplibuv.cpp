// kore.io soup
#include <unistd.h>
#include "deplibuv.hpp"
#include <iostream>
#include <cstdlib>

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
uv_loop_t* deplibuv::loop{nullptr};


void deplibuv::classinit(){
	std::cout << "Entering deplibuv::classinit()\n";
int err;
	deplibuv::loop=new uv_loop_t;
	err=uv_loop_init(deplibuv::loop);
	if(err !=0){
	std::cout << red "Libuv init failed.\n" rst;
	std::abort();
	}
	
}
// just another dummy method while developing
void deplibuv::display(char*text){std::cout << text << std::endl;};


void deplibuv::classdestroy(){
	printf("classsssssssssssss desproy\n");
if(deplibuv::loop==nullptr){ 
std::cout << red "Loop was not allocated!\n" rst;
std::abort();
}

uv_loop_close(deplibuv::loop);
delete deplibuv::loop;
std::cout << yellow "Look ma, loop is destroyd." rst << std::endl;
}


void deplibuv::printversion(){
std::cout <<  green "Libuv version:  " rst << uv_version_string() << std::endl;
}

void deplibuv::runloop(){
if(deplibuv::loop==nullptr){
std::cout << red "loop was not allocated 2!\n" rst << std::endl;
std::abort();
}
	
std::cout << green "Loop is allocated successfully" rst << std::endl;
uv_run(deplibuv::loop,UV_RUN_DEFAULT);
}

// C wrapper
void deplibuv_printversion(){
deplibuv::printversion();
}
void class_init(){
deplibuv::classinit();
}
void run_loop(){
deplibuv::runloop();
}
void class_destroy(){
deplibuv::classdestroy();
}
uv_loop_t*get_loopi(){
return deplibuv::getloop();
}
