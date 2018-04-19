#include <unistd.h>
#include "deplibuv.hpp"
#include <iostream>
#include <cstdlib>


uv_loop_t* deplibuv::loop{nullptr};


void deplibuv::classinit(){
	std::cout << "Entering deplibuv::classinit()\n";
int err;
	deplibuv::loop=new uv_loop_t;
	err=uv_loop_init(deplibuv::loop);
	if(err !=0){
	std::cout << "Libuv init failed.\n";
	std::abort();
	}
	
}
// just another dummy method while developing
void deplibuv::display(char*text){std::cout << text << std::endl;};


void deplibuv::classdestroy(){
if(deplibuv::loop==nullptr){ 
std::cout << "Loop was not allocated!\n";
std::abort();
}

uv_loop_close(deplibuv::loop);
delete deplibuv::loop;
std::cout << "Look ma, loop is destroyd." << std::endl;
}
void deplibuv::printversion(){
std::cout << "Libuv version:  " << uv_version_string() << std::endl;
}
void deplibuv::runloop(){
if(deplibuv::loop==nullptr){
std::cout << "loop was not allocated 2!\n"<< std::endl;
std::abort();
}
	
std::cout << "Loop is allocated successfully" << std::endl;
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
