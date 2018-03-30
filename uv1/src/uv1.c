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
int pipe_reader(struct kore_task*);
void pipe_data_available(struct kore_task*);
void*on_progress(uv_callback_t*,void*);
void*on_result(uv_callback_t*,void*);
void*on_result2(uv_callback_t*,void*);
void * stop_worker_cb(uv_callback_t*,void*);

void on_walk(uv_handle_t*,void*);

void fake_download(uv_work_t*);
void after(uv_work_t*,int);

void * on_sum(uv_callback_t*,void*);

void *print_progress(uv_callback_t*,void*);

struct kore_task pipe_task;
uv_callback_t progress;
uv_callback_t cb_result,cb_result2;
uv_callback_t cb_sum;
uv_callback_t stop_worker;
uv_callback_t send_data,send_data2;
uv_barrier_t barrier;
uv_loop_t *loop=NULL;
//uv_async_t async;
uv_work_t lreq;
void*on_data(uv_callback_t*,void*);
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
uv_callback_fire(&progress,(void*)510,NULL);
uv_queue_work(loop,&lreq,fake_download,after);
http_response(req,200,asset_frontend_html,asset_len_frontend_html);
kore_log(LOG_NOTICE,"http request should be sent");
return (KORE_RESULT_OK);
}

void*on_data(uv_callback_t*handle,void*data){
	printf("\nON_DATAAAAAAAAAAAAAAAA!!! : %s\n",(char*)data);
char *s=(char*)data;
return s;
}

void *print_progress(uv_callback_t*handle,void*data){
double p=*((double*)data);
fprintf(stderr,"downloaded: %.2f%%\n",p);
return data;
}
void*on_sum(uv_callback_t*callback,void*data){
printf("\nON_SUM %s\n",(char*)data);
	//uv_callback_fire(&send_data,"lumi",&cb_result);
	//uv_callback_init(&loop,&send_data,on_data,UV_DEFAULT);
	uv_callback_fire(&progress,(void*)400,NULL);
	char*m;
	m=(char*)data;
return m;
}

void * on_progress(uv_callback_t*handle,void*value){
printf("progress: %d\n",(int)value);
//uv_queue_work(loop,&lreq,fake_download,after);
return NULL;
}
void * on_result(uv_callback_t*callback,void*data){
printf("on RESULT: %s\n",(char*)data);
return (char*)data;
}

void * on_result2(uv_callback_t*callback,void*data){
double p=*((double*)data);
//printf("on RESULT2: %s\n",(char*)data);
printf("RESULT2: !!!! downloaded: %.2f%%\n",p);
return (char*)data;
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
//uv_callback_fire(&send_data,"lumi",&cb_result);
uv_callback_init(loop,&send_data,on_data,UV_DEFAULT);
	uv_callback_init(loop,&send_data2,print_progress,UV_DEFAULT);
uv_callback_fire(&send_data,"lumi",&cb_result);
	sleep(1);
uv_callback_fire(&send_data,"fake_download 1?",&cb_result);
kore_log(LOG_NOTICE,"ON FAKE_DOWNLOAD 1\n");
sleep(1);
	uv_callback_fire(&send_data,"fake_download 2?",&cb_result);
kore_log(LOG_NOTICE,"ON FAKE_DOWNLOAD 2\n");
	
	int size=*((int*)req->data);
	printf("SIZE!!!!!!!!!!!!!!!!!!!!!!!!!: %d\n",size);
	
	int d=0;
	double p;
	while(d<size){
	p=d*100.0/size;
		fprintf(stderr,"a?\n");
		//async.data=(void*)&p;
		//uv_async_send(&async);
		uv_callback_fire(&send_data2,(void*)&p,&cb_result2);
		sleep(1);
		d+=(200+random())%1000;
	}

}
void after(uv_work_t*req,int status){
kore_log(LOG_NOTICE,"ON_AFTER OCCURED");
//uv_close((uv_handle_t*)&async,NULL);
}

int pipe_reader(struct kore_task*t){
kore_log(LOG_NOTICE,"task created");
int rc;
loop=uv_default_loop();
//uv_barrier_init(&barrier,2);
//uv_barrier_wait(&barrier);
//uv_loop_t loop;
uv_callback_init(loop,&progress,on_progress,UV_COALESCE);	
uv_callback_fire(&progress,(void*)20,NULL);
//uv_callback_fire(&progress,"dama", &cb_result);
//uv_queue_work(loop,&lreq,fake_download,after);
//uv_callback_init(loop,&send_data,on_data,UV_DEFAULT);
 rc=uv_callback_init(loop,&cb_result,on_result,UV_DEFAULT);
printf("\nrc2 %d\n",rc);
	
rc=uv_callback_init(loop,&cb_result2,on_result2,UV_DEFAULT);
printf("\nrc2o %d\n",rc);
	
//uv_callback_fire(&send_data,"vumi",&cb_result);
rc=uv_callback_init(loop,&cb_sum,on_sum,UV_DEFAULT);
	printf("\nrc3 %d\n",rc);
	uv_callback_fire(&cb_sum,"damS", NULL);
	//kore_task_channel_write(t,"mama\0",5);
	int size=1020;
	lreq.data=(void*)&size;
	//sleep(1);
	/*
	rc=uv_callback_init(loop,&stop_worker,stop_worker_cb,UV_COALESCE);
	printf("\nrc4 %d\n",rc);
	uv_callback_fire(&stop_worker,NULL,NULL);
	*/
	//uv_walk(loop,on_walk,NULL);
	
	//uv_async_init(loop,&async,print_progress);
	uv_run(loop,UV_RUN_DEFAULT);
	kore_task_channel_write(t,"mama\0",5);
	sleep(1);
	//uv_loop_close(loop);
	return (KORE_RESULT_OK);
}
void pipe_data_available(struct kore_task*t){
if(kore_task_finished(t)){kore_log(LOG_WARNING,"a task is finished.");return;}
   u_int8_t buf[BUFSIZ];
   size_t len=kore_task_channel_read(t,buf,sizeof(buf));
   if(len>sizeof(buf)) kore_log(LOG_WARNING,"truncated data from task.");
   kore_log(LOG_NOTICE,"msg %s",buf);
   
}