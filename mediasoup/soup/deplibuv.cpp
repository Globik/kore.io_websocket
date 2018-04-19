#include <unistd.h>
#include "deplibuv.hpp"
#include <iostream>
#include <cstdlib>


uv_loop_t* deplibuv::loop{nullptr};


void deplibuv::classinit(){
	std::cout << "here class init\n";
int err;
	deplibuv::loop=new uv_loop_t;
	err=uv_loop_init(deplibuv::loop);
	if(err !=0){
	std::cout << "libuv init failed\n";
	std::abort();
	}
	
}
void deplibuv::display(char*text){std::cout << text << std::endl;};


void deplibuv::classdestroy(){
if(deplibuv::loop==nullptr){ 
std::cout << "loop was not allocated.\n";
std::abort();
}
//uv_stop(deplibuv::loop);
//	usleep(100000);

uv_loop_close(deplibuv::loop);
delete deplibuv::loop;
std::cout<<"Loop was destroyd?"<<std::endl;
}
void deplibuv::printversion(){
//std::printf("version %s\n", uv_version_string());
	std::cout << "version:  " << uv_version_string() << std::endl;
}
void deplibuv::runloop(){
if(deplibuv::loop==nullptr){
std::cout << "loop was not allocated 2.\n"<< std::endl;
std::abort();
}
	
	
	std::cout << "Loop was allocated?" << std::endl;
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
