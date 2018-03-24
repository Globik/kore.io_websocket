#include <uv.h>
void idle_cb(uv_idle_t*handle){
static int64_t count=-1;
	count++;
	if((count % 10000)==0) printf(".\n");
	if(count >=500000) {printf("Stop here!\n");uv_idle_stop(handle);}
}
int main(){
uv_idle_t idle_handle;
	uv_loop_t *loop=uv_default_loop();
	uv_idle_init(loop,&idle_handle);
	uv_idle_start(&idle_handle,idle_cb);
	uv_run(loop,UV_RUN_DEFAULT);
	return 0;
}
/*
gcc -g -o m m.c `pkg-config --libs libuv`
*/