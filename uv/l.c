// https://github.com/Elzair/libuv-examples
// gcc -o l l.c `pkg-config --libs libuv`
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>
uv_loop_t *loop;
uv_async_t async;
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
fprintf(stderr,"download complete\n");
uv_close((uv_handle_t*)&async,NULL);
}
int main(){
loop=uv_default_loop();
	uv_work_t req;
	int size=1020;
	req.data=(void*)&size;
	uv_async_init(loop,&async,print_progress);
	uv_queue_work(loop,&req,fake_download,after);
	//uv_queue_work(loop,&req,fake_download,after);
	
	return uv_run(loop,UV_RUN_DEFAULT);
}