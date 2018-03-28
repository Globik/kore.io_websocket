#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
#include <uv.h>
#include "assets.h"
int init(int);
int page(struct http_request*);
int pipe_reader(struct kore_task*);
void pipe_data_available(struct kore_task*);
uv_loop_t *loop=NULL;
uv_async_t async;
uv_work_t lreq;
void print_progress(uv_async_t*);
void fake_download(uv_work_t*);
void after(uv_work_t*,int);
//void idle_cb(uv_idle_t*);
struct kore_task pipe_task;
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
	if(loop !=NULL){kore_log(LOG_NOTICE,"loop here");
	uv_queue_work(loop,&lreq,fake_download,after);
	}
	kore_log(LOG_NOTICE,"loop after");
http_response(req,200,asset_frontend_html,asset_len_frontend_html);
kore_log(LOG_NOTICE,"http request should be sent");
return (KORE_RESULT_OK);
}
#define MAX_REPORT_LEN 1024
#define check(r,msg) if(r){ \
printf("%s : [%s(%d): %s]\n",msg,uv_err_name((r)),(int) r,uv_strerror((r))); \
exit(1); \
}
#define BUF_SIZE 255
//static const char *filename="/home/globik/kore.io_websocket/uv/m.c";


void print_progress(uv_async_t*handle/*,int status*/){
double p=*((double*)handle->data);
	fprintf(stderr,"downloaded: %.2f%%\n",p);
}
void fake_download(uv_work_t*req){
int size=*((int*)req->data);
	int d=0;
	double p;
	while(d<size){
	p=d*100.0/size;
		fprintf(stderr,"a?\n");
		async.data=(void*)&p;
		uv_async_send(&async);
		sleep(1);
		d+=(200+random())%1000;
	}
}
void after(uv_work_t*req,int status){
fprintf(stderr,"download complete %d status: %d\n",*((int*)req->data),status);
//uv_close((uv_handle_t*)&async,NULL);
}
void open_cb(uv_fs_t*);
void read_cb(uv_fs_t*);
void close_cb(uv_fs_t*);
void binit(uv_loop_t*,struct kore_task*);
void mic(struct kore_task*);
typedef struct context_struct{
uv_fs_t *open_req;
	uv_fs_t *read_req;
	uv_buf_t iov;
	int a;
	struct kore_task *mt;
}context_t;
void open_cb(uv_fs_t*open_req){
int r=0;
if(open_req->result < 0)check(open_req->result,"uv_fs_open callback");
context_t* context=open_req->data;
kore_log(LOG_NOTICE,"integer a %d\n",context->a);
kore_task_channel_write(context->mt,"OPEN\0",5);
size_t buf_len=sizeof(char*)*BUF_SIZE;
char*buf=malloc(buf_len);
context->iov=uv_buf_init(buf,buf_len);
						 
uv_fs_t*read_req=malloc(sizeof(uv_fs_t));
context->read_req=read_req;
read_req->data=context;
						 
r=uv_fs_read(uv_default_loop(),read_req,open_req->result,&context->iov,1,0,read_cb);
if(r<0) check(r,"uv_fs_read");
}



void read_cb(uv_fs_t*read_req){
int r=0;
	if(read_req->result < 0) check(read_req->result,"uv_fs_read callback");
	context_t*context=read_req->data;
	kore_log(LOG_NOTICE,"integer a=%d\n",context->a);
	kore_task_channel_write(context->mt,"READ\0",5);
	kore_log(LOG_NOTICE,"File: %s\n",context->iov.base);
	free(context->iov.base);
	uv_fs_t*close_req=malloc(sizeof(uv_fs_t));
	close_req->data=context;
	r=uv_fs_close(uv_default_loop(),close_req,context->open_req->result,close_cb);
	if(r<0) check(r,"uv_fs_close");
}
void mic(struct kore_task*l){
kore_task_channel_write(l,"FFMA\0",5);
}
void close_cb(uv_fs_t*close_req){
if(close_req->result < 0) check(close_req->result,"uv_fs_close callback");
context_t*context=close_req->data;
mic(context->mt);
	//kore_log(LOG_NOTICE,"integer a==%d\n",context->a);
	//kore_task_channel_write(context->mt,"CLOS\0",5);
	sleep(1);
	uv_fs_req_cleanup(context->open_req);
	uv_fs_req_cleanup(context->read_req);
	uv_fs_req_cleanup(close_req);
	context->mt=NULL;
	free(context);
}
/*
void binit(uv_loop_t*loop,struct kore_task*t){
int r;
uv_fs_t*open_req=malloc(sizeof(uv_fs_t));
context_t*context=malloc(sizeof(context_t));
context->open_req=open_req;
context->a=666;
context->mt=t;
open_req->data=context;
r=uv_fs_open(loop,open_req,filename,O_RDONLY,S_IRUSR,open_cb);
if(r<0)check(r,"uv_fs_open");
}

*/
/*
void idle_cb(uv_idle_t*handle){
static int64_t count=-1;
	count++;
	if((count % 10000)==0) kore_log(LOG_NOTICE,".\n");
	if(count >=500000) {kore_log(LOG_NOTICE,"Stop here!\n");uv_idle_stop(handle);}
}
*/
int pipe_reader(struct kore_task*t){
	/*
	uv_idle_t idle_handle;
	uv_loop_t *loop=uv_default_loop();
	uv_idle_init(loop,&idle_handle);
	uv_idle_start(&idle_handle,idle_cb);
	uv_run(loop,UV_RUN_DEFAULT);
	//return 0;
	*/
	/*
uv_loop_t*loop=uv_default_loop();
binit(loop,t);
uv_run(loop,UV_RUN_DEFAULT);
	*/
	
	loop=uv_default_loop();
//	uv_work_t req;
	int size=1020;
	lreq.data=(void*)&size;
	uv_async_init(loop,&async,print_progress);
	//uv_queue_work(loop,&req,fake_download,after);
	uv_run(loop,UV_RUN_DEFAULT);
	kore_task_channel_write(t,"mama\0",5);
	int b=uv_loop_close(loop);
kore_log(LOG_NOTICE,"b: %d",b);
	loop=NULL;
	//sleep(1);
//kore_task_channel_write(t,"mama\0",5);
	
	return (KORE_RESULT_OK);
}
void pipe_data_available(struct kore_task*t){
if(kore_task_finished(t)){kore_log(LOG_WARNING,"a task is finished.");return;}
   u_int8_t buf[BUFSIZ];
   size_t len=kore_task_channel_read(t,buf,sizeof(buf));
   if(len>sizeof(buf)) kore_log(LOG_WARNING,"truncated data from task.");
   kore_log(LOG_NOTICE,"msg %s",buf);
   
}