#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <memory.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
#include <uv.h>

#include "assets.h"
#include "uv_callback.h"
int init(int);
int page(struct http_request*);
int page2(struct http_request*);
int pipe_reader(struct kore_task*);
void pipe_data_available(struct kore_task*);
void*on_result2(uv_callback_t*,void*);
void * stop_worker_cb(uv_callback_t*,void*);

void on_walk(uv_handle_t*,void*);

void fake_download(uv_work_t*);
void after(uv_work_t*,int);


void *print_progress(uv_callback_t*,void*);

struct kore_task pipe_task;

uv_callback_t cb_result2, send_data2, stop_worker;
uv_loop_t *loop=NULL;
uv_work_t lreq;

int init(int state){
if(state==KORE_MODULE_UNLOAD) return (KORE_RESULT_ERROR);
	//if(worker->id !=1) return (KORE_RESULT_OK);
	
	kore_task_create(&pipe_task,pipe_reader);
	kore_task_bind_callback(&pipe_task,pipe_data_available);
	kore_task_run(&pipe_task);
	return (KORE_RESULT_OK);
}
int page(struct http_request*req){
http_response_header(req,"content-type","text/html");

uv_queue_work(loop,&lreq,fake_download,after);
http_response(req,200,asset_frontend_html,asset_len_frontend_html);
kore_log(LOG_NOTICE,"http request should be sent");
return (KORE_RESULT_OK);
}

int page2(struct http_request*req){
http_response_header(req,"content-type","text/html");

http_response(req,200,asset_front2_html,asset_len_front2_html);
kore_log(LOG_NOTICE,"front2.html");
return (KORE_RESULT_OK);
}


void *print_progress(uv_callback_t*handle,void*data){
struct kore_task*t=(struct kore_task*)data;
kore_log(LOG_NOTICE,"print_progress() ?");
kore_task_channel_write(t,"from print_progress\0", 20);
return data;
}

void * on_result2(uv_callback_t*callback,void*data){
struct kore_task*t=(struct kore_task*)data;
kore_log(LOG_NOTICE,"on_result2() ?");
kore_task_channel_write(t,"from on_result2\0", 16);
return data;
}

void on_walk(uv_handle_t*handle,void*arg){
printf("\nON_WALK\n");
uv_close(handle,NULL);
	
}

void * stop_worker_cb(uv_callback_t*handle,void*arg){
	printf("\n on stop worker cb\n");
uv_stop(((uv_handle_t*)handle)->loop);
return NULL;
}

void fake_download(uv_work_t*req){
struct kore_task*t=(struct kore_task*)req->data;
sleep(1);
if(t==NULL){kore_log(LOG_NOTICE,"t is NULL");return;}
kore_log(LOG_NOTICE,"fake_download() ?");
kore_task_channel_write(t,"from fake_download\0", 19);	
uv_callback_init(req->loop,&send_data2, print_progress, UV_DEFAULT);
uv_callback_fire(&send_data2, req->data, &cb_result2);

}
void after(uv_work_t*req,int status){
struct kore_task*t=(struct kore_task*)req->data;
kore_log(LOG_NOTICE,"ON_AFTER OCCURED");
kore_task_channel_write(t,"from after\0", 11);
}

int pipe_reader(struct kore_task*t){
kore_log(LOG_NOTICE,"A task created");
int rc;
loop=uv_default_loop();
	
rc=uv_callback_init(loop,&cb_result2,on_result2,UV_DEFAULT);
kore_log(LOG_INFO,"uv_callback_t cb_result2 init: %d\n", rc);
lreq.data=t;
	/*
	rc=uv_callback_init(loop,&stop_worker,stop_worker_cb,UV_COALESCE);
	printf("\nrc4 %d\n",rc);
	uv_callback_fire(&stop_worker,NULL,NULL);
	*/
	//uv_walk(loop,on_walk,NULL);
	
	uv_run(loop,UV_RUN_DEFAULT);
	kore_task_channel_write(t,"mama\0",5);
	//sleep(1);
	//uv_loop_close(loop);
	return (KORE_RESULT_OK);
}
void pipe_data_available(struct kore_task*t){
if(kore_task_finished(t)){kore_log(LOG_WARNING,"a task is finished.");return;}
   u_int8_t buf[BUFSIZ];
   size_t len=kore_task_channel_read(t,buf,sizeof(buf));
   if(len>sizeof(buf)) kore_log(LOG_WARNING,"truncated data message from task.");
   kore_log(LOG_NOTICE,"A message came: %s",buf);
   
}