#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//#include <memory.h>
#include <signal.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
//#include <uv.h>

#include "assets.h"
//#include "uv_callback.h"
#include "libuv.hpp"



int init(int);
int run_curl(struct kore_task*);
int page(struct http_request*);
int page2(struct http_request*);
int pipe_reader(struct kore_task*);
void pipe_data_available(struct kore_task*);

void * stop_worker_cb(uv_callback_t*,void*);
void * on_bus(uv_callback_t*, void*);
//void j_han_sig(int);
//void j_exit(void);
void han(void);


struct kore_task pipe_task;
struct rstate{
	struct kore_task task;
};

uv_callback_t bus, stop_worker, to_cpp;

void han(){
int rc=uv_callback_fire(&stop_worker,NULL,NULL);
kore_log(LOG_NOTICE,"rc stop_worker fire %d",rc);
usleep(20000);
kore_log(LOG_NOTICE,"at exit han()");
}
void kore_worker_configure(){
kore_log(LOG_NOTICE,"worker configure");
atexit(han);
}
int init(int state){
if(state==KORE_MODULE_UNLOAD) return (KORE_RESULT_ERROR);
	//if(worker->id !=1) return (KORE_RESULT_OK); //if  cpu workers great than 1 comment it out for a dedicated task
	
	kore_task_create(&pipe_task,pipe_reader);
	kore_task_bind_callback(&pipe_task,pipe_data_available);
	kore_task_run(&pipe_task);
	return (KORE_RESULT_OK);
}
int page(struct http_request*req){
http_response_header(req,"content-type","text/html");
// Fire callback to trans a message to libuv.cpp class and check if it blocks kore workflow
// as a "request" mechanism from the kore's world to the libuv.cpp class.
int rc=uv_callback_fire(&to_cpp,(void*)"CPP IS OK?", NULL);
// if 0 then OK
kore_log(LOG_NOTICE,"rc to_cpp fire: %d\n",rc);
http_response(req,200,asset_frontend_html,asset_len_frontend_html);
kore_log(LOG_NOTICE,"http request should be sent");
return (KORE_RESULT_OK);
}

int page2(struct http_request*req){
// open front2[html] in a new browser's tab to check if the uv_loop blocks the frontend_html
	char result[64];
	u_int32_t len;
struct rstate * state;
	if(req->hdlr_extra==NULL){
	state=kore_malloc(sizeof(*state));
		req->hdlr_extra=state;
		kore_task_create(&state->task,run_curl);
		kore_task_bind_request(&state->task,req);
		kore_task_run(&state->task);
		kore_task_channel_write(&state->task,"Lama\0",5);
		return (KORE_RESULT_RETRY);
	}else{
	state=req->hdlr_extra;
	}
	if(kore_task_state(&state->task) !=KORE_TASK_STATE_FINISHED){
	http_request_sleep(req);
		return (KORE_RESULT_RETRY);
	}
	if(kore_task_result(&state->task) !=KORE_RESULT_OK){
		kore_task_destroy(&state->task);
		http_response(req,500,NULL,0);
		return (KORE_RESULT_OK);
	}
	len=kore_task_channel_read(&state->task,result,sizeof(result));
	if(len>sizeof(result)){
		kore_log(LOG_INFO,"len > sizeof(result) in page2");
	http_response(req,500,NULL,0);
	}else{
		kore_log(LOG_INFO,"Result came: %s",result);
http_response_header(req,"content-type","text/html");
http_response(req,200,asset_front2_html,asset_len_front2_html);
kore_log(LOG_NOTICE,"front2.html");
	}
	kore_task_destroy(&state->task);
return (KORE_RESULT_OK);
}

int run_curl(struct kore_task*t){
size_t len;
char user[64];
	len=kore_task_channel_read(t,user,sizeof(user));
	if(len>sizeof(user)){
	kore_log(LOG_INFO,"len > sizeof(user)");
		return (KORE_RESULT_ERROR);
	}
	kore_log(LOG_INFO,"data in run_curl read: %s",user);
	kore_task_channel_write(t,"Puma\0",5);
	return (KORE_RESULT_OK);
}



void * stop_worker_cb(uv_callback_t*handle,void*arg){
kore_log(LOG_NOTICE,"\n on stop worker cb\n");
uv_stop(((uv_handle_t*)handle)->loop);
// s
return NULL;
}

void * on_bus(uv_callback_t*cb, void*data){
kore_log(LOG_NOTICE,"ON_BUS occured. %s\n", (char*)data);
return NULL;
}

void*on_from_cpp(uv_callback_t*handle,void*data){
// should be came from libuv.cpp class. As a "notification" mechanism from libuv's world to the kore's world.
// TODO: check if two way(bidirectional) communication callbacks are blocking each other in the libuv loop thread
kore_log(LOG_NOTICE,"ON_FROM_CPP occured. %s\n",(char*)data);
return NULL;
}


int pipe_reader(struct kore_task*t){
kore_log(LOG_NOTICE,"A task created");
kore_task_channel_write(t,"mama\0",5);
int rc;

// init libuv loop
ini();
// print libuv version
printi();
// right in place , in this file
rc=uv_callback_init(get_loopi(), &bus, on_bus, UV_DEFAULT);
// just for fun
kore_log(LOG_NOTICE,"rc on_bus init: %d\n",rc);
rc=uv_callback_fire(&bus,(void*)"MAMA", NULL);
kore_log(LOG_NOTICE, "rc fire: %d\n",rc);
// init a trans to send a message to libuv.cpp class	
rc=uv_callback_init(get_loopi(), &to_cpp, trans_to_cpp, UV_DEFAULT);
kore_log(LOG_NOTICE, "rc to_cpp init: %d\n",rc);


	
rc=uv_callback_init(get_loopi(),&stop_worker,stop_worker_cb,UV_COALESCE);
kore_log(LOG_NOTICE,"rc init stop_worker: %d\n",rc);
runi();
kore_task_channel_write(t,"FUCK\0",5);
kore_log(LOG_NOTICE,"END?");
destri();
kore_log(LOG_NOTICE,"Bye. *******\n");
return (KORE_RESULT_OK);
}
void pipe_data_available(struct kore_task*t){
if(kore_task_finished(t)){kore_log(LOG_WARNING,"a task is finished.");return;}
   u_int8_t buf[BUFSIZ];
   size_t len=kore_task_channel_read(t,buf,sizeof(buf));
   if(len>sizeof(buf)) kore_log(LOG_WARNING,"truncated data message from task.");
   kore_log(LOG_NOTICE,"A message came: %s",buf);
   
}