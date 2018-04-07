///home/globik/kore-mediasoup/training/luvi/l/ 
#include "libuv.hpp"
#include <iostream>
#include <cstdlib>
#include <unistd.h>

uv_loop_t* deplibuv::loop{nullptr};

uv_callback_t from_cpp;

void deplibuv::classinit(){
	std::cout << "here class init\n";
int err;
	deplibuv::loop=new uv_loop_t;
	err=uv_loop_init(deplibuv::loop);
	if(err !=0){
	std::cout << "libuv init failed\n";
	exit(1);
	}
	int rc=uv_callback_init(deplibuv::loop,&from_cpp, on_from_cpp, UV_DEFAULT);
	std::cout << "in deplibuv.cpp rc2: " << rc << std::endl;
}


void deplibuv::classdestroy(){
if(deplibuv::loop==nullptr){ 
std::cout << "loop was not allocated.\n";

exit(1);	
}
//uv_stop(deplibuv::loop);
uv_loop_close(deplibuv::loop);
delete deplibuv::loop;
std::cout<<"Loop was destroyd?"<<std::endl;

}
void ini(){
deplibuv::classinit();
}
void printi(){
deplibuv::printversion();
}
void runi(){
deplibuv::runloop();
}
void destri(){
deplibuv::classdestroy();
}
void deplibuv::printversion(){
std::cout << "version:  " << uv_version_string() << std::endl;
}
void*deplibuv::on_to_cpp(uv_callback_t*callback,void*data){
std::printf("ON_TO_CPP occured: %s\n",(char*)data);
sleep(6);
std::printf("AFTER SLEEP.\n");
return nullptr;
}
void*trans_to_cpp(uv_callback_t*callback,void*data){
deplibuv::on_to_cpp(callback,data);
}
uv_loop_t*get_loopi(){
return deplibuv::getloop();
}
void deplibuv::runloop(){
if(deplibuv::loop==nullptr){
std::cout << "loop was not allocated 2.\n"<< std::endl;
exit(1);
}
	
int rc=uv_callback_fire(&from_cpp,(void*)"PAPA", NULL);
	std::cout << "rc fire from_cpp " << rc << std::endl;
	
	std::cout << "Loop was allocated?" << std::endl;
	uv_run(deplibuv::loop,UV_RUN_DEFAULT);
}