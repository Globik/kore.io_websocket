#include <uv.h>
#include <stdio.h>
#include <stdlib.h>


void free_write_req(uv_handle_t*handle){
printf("free handle\n");
	free(handle);
}
void on_write(uv_write_t*req,int s){
	printf("on write enter\n");
if(s<0){
fprintf(stderr,"write error %s\n",uv_err_name(s));
}else{
	puts("done");
uv_close((uv_handle_t*)req->handle,free_write_req);
}
	printf("free req\n");
	free(req);
//free_write_req(req);
}
void on_connect(uv_connect_t*con,int s){
if(s<0){printf("failed on_connect to server.\n");}else{
	printf("connected! sending msg...\n");
	uv_write_t*out=(uv_write_t*)malloc(sizeof(uv_write_t));
	if(out==NULL)printf("NULLL\n");
	uv_buf_t wbuf=uv_buf_init("hell",4);
	uv_write(out,(uv_stream_t*)con->handle,&wbuf,1,on_write);
}
	free(con);
}
// gcc -o client client.c `pkg-config --libs libuv`
int main(){
uv_loop_t*loop=uv_default_loop();
	uv_pipe_t*handle=(uv_pipe_t*)malloc(sizeof(uv_pipe_t));
	uv_connect_t*connect=(uv_connect_t*)malloc(sizeof(uv_connect_t));
	uv_pipe_init(loop,handle,0);
	uv_pipe_open(handle,socket(PF_UNIX,SOCK_STREAM,0));
	uv_pipe_connect(connect,handle,"echo.sock2",on_connect);
	uv_run(loop,UV_RUN_DEFAULT);
	return 0;
}