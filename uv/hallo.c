#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <uv.h>
#define derror(msg,code) do { \
fprintf(stderr,"%s: [%s: %s]\n",msg,uv_err_name((code)),uv_strerror((code))); \
assert(0); \
} while(0);
#define noipc 0
uv_loop_t*loop;
void alloc_cb(uv_handle_t *handle,size_t size,uv_buf_t*buf){
buf->base=malloc(size);
assert(buf->base !=NULL);
buf->len=size;
}
void write_cb(uv_write_t*req,int status){
if(status<0)derror("async write",status);
	char*base=(char*)req->data;
	free(base);
	free(req);
}
void read_cb(uv_stream_t*client,ssize_t nread,const uv_buf_t*buf){
	int r;
if(nread<=0 && buf->base !=NULL){
	free(buf->base);
fprintf(stderr,"read error: [%s: %s]\n",uv_err_name((nread)),uv_strerror((nread)));
uv_close((uv_handle_t*)client,NULL);
	return;
}
	
	
uv_write_t*req=(uv_write_t*)malloc(sizeof(uv_write_t));
	req->data=(void*)buf->base;
	uv_write(req,client,buf,1,write_cb);
}
void connect_cb(uv_stream_t*server,int status){
int r;
	if(status){
	fprintf(stderr,"connection error: [%s: %s]\n",uv_err_name((status)),uv_strerror((status)));
		return;
	}
	printf("CLient connected!\n");
	uv_pipe_t*client=malloc(sizeof(uv_pipe_t));
	r=uv_pipe_init(uv_default_loop(),client,noipc);
	if(r)derror("initializing client pipe",r);
	r=uv_accept(server,(uv_stream_t*)client);
	if(r==0){
	uv_read_start((uv_stream_t*)client,alloc_cb,read_cb);
	}else{
	printf("closing\n");
	uv_close((uv_handle_t*)client,NULL);
	}

}
void sigint_cb(int sig){
int r;
	uv_fs_t req;
	r=uv_fs_unlink(loop,&req,"echo.sock2",NULL);
	if(r)derror("unlinking echo.sock",r);
	exit(0);
}
int main(){
	int r;
loop=uv_default_loop();
signal(SIGINT,sigint_cb);
	uv_pipe_t server;
	uv_pipe_init(loop,&server,noipc);
	printf("quitting\n");
	r=uv_pipe_bind(&server,"echo.sock2");
	if(r)derror("binding to echo.sock2",r);
	r=uv_listen((uv_stream_t*)&server,128,connect_cb);
	if(r)derror("listening on socket",r);
	return uv_run(loop,UV_RUN_DEFAULT);
	//uv_loop_close(loop);
	//free(loop);
	//return 0;
}
// gcc -o h hallo.c `pkg-config --libs libuv`