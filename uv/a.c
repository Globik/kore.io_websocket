#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <uv.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#define MAX_REPORT_LEN 1024
#define check(r,msg) if(r){ \
printf("%s : [%s(%d): %s]\n",msg,uv_err_name((r)),(int) r,uv_strerror((r))); \
exit(1); \
}
#define BUF_SIZE 255
static const char *filename="/home/globik/kore.io_websocket/uv/m.c";
void open_cb(uv_fs_t*);
void read_cb(uv_fs_t*);
void close_cb(uv_fs_t*);
typedef struct context_struct{
uv_fs_t *open_req;
	uv_fs_t *read_req;
	uv_buf_t iov;
	int a;
}context_t;
void open_cb(uv_fs_t*open_req){
int r=0;
if(open_req->result < 0)check(open_req->result,"uv_fs_open callback");
context_t* context=open_req->data;
	printf("integer a %d\n",context->a);
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
	printf("integer a=%d\n",context->a);
	printf("%s\n",context->iov.base);
	free(context->iov.base);
	uv_fs_t*close_req=malloc(sizeof(uv_fs_t));
	close_req->data=context;
	r=uv_fs_close(uv_default_loop(),close_req,context->open_req->result,close_cb);
	if(r<0) check(r,"uv_fs_close");
}

void close_cb(uv_fs_t*close_req){
if(close_req->result < 0) check(close_req->result,"uv_fs_close callback");
	context_t*context=close_req->data;
	printf("integer a==%d\n",context->a);
	uv_fs_req_cleanup(context->open_req);
	uv_fs_req_cleanup(context->read_req);
	uv_fs_req_cleanup(close_req);
	free(context);
}

void init(uv_loop_t*loop){
int r;
	uv_fs_t*open_req=malloc(sizeof(uv_fs_t));
	
	context_t*context=malloc(sizeof(context_t));
	context->open_req=open_req;
	context->a=666;
	open_req->data=context;
	r=uv_fs_open(loop,open_req,filename,O_RDONLY,S_IRUSR,open_cb);
	if(r<0)check(r,"uv_fs_open");
}

int main(){
uv_loop_t*loop=uv_default_loop();
init(loop);
uv_run(loop,UV_RUN_DEFAULT);
return 0;
}
/*

gcc -o a a.c `pkg-config --libs libuv`

*/



















