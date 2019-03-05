//#include <kore/kore.h>
#include <kore/http.h>
#include "janus.h"
#include <kore/tasks.h>
#include <jansson.h>
#include "assets.h"
//.fr
int		page(struct http_request *);
int page_ws_connect(struct http_request*);
void websocket_connect(struct connection*);
void websocket_disconnect(struct connection*);
void websocket_message(struct connection*,u_int8_t,void*,size_t);
int init(int);
int janus_task(struct kore_task*);
void data_available(struct kore_task*);
json_t*load_json(const char*, size_t);
void kore_worker_configure(){
printf("CONFIGURE WORKER!\n");	
}

void kore_worker_teardown(void){
kore_log(LOG_INFO,"kore_worker_teardown");
printf("\n*** TEARDOWN! ***\n");	

g_atomic_int_inc(&stop);
usleep(300000);
}
struct kore_task task;
int init(int state){
if(state==KORE_MODULE_UNLOAD)return (KORE_RESULT_ERROR);	
kore_task_create(&task, janus_task);
kore_task_bind_callback(&task, data_available);
kore_task_run(&task, 0);
return (KORE_RESULT_OK);
}

json_t *load_json(const char*text, size_t buflen){
json_t *root;
json_error_t error;
root=json_loadb(text, buflen, 0, &error);
if(root){return root;}else{
kore_log(LOG_INFO,"json buffer parse err: %d %s",error.line,error.text);
return (json_t*)0;	
}	
}

int page_ws_connect(struct http_request*req){
	g_print("websocket connected %s %p\n", req->path, req);
	kore_websocket_handshake(req,"websocket_connect", "websocket_message", "websocket_disconnect");
	return (KORE_RESULT_OK);
}

int
page(struct http_request *req)
{
	//http_response(req, 200, NULL, 0);
	http_response_header(req,"content-type","text/html");
	//Lorenzo's shit here must be but with no luck; bug!
	// so I must myself all do here
	http_response(req,200, asset_index_html, asset_len_index_html);
	return (KORE_RESULT_OK);
}

void websocket_connect(struct connection*c){
g_print("websocket connected %p\n",c);	
}
void websocket_disconnect(struct connection*c){
//kore_log(LOG_INFO,"websocket disconnected %p", c);	
//JANUS_LOG(LOG_VERB, "websocket disconnected %p\n",c);
g_print("g_print: websocket disconnected %p\n",c);
}
void websocket_message(struct connection*c,u_int8_t op, void* vdata, size_t vlen){
//kore_log(LOG_INFO,"message");
//printf("message: \n");
//JANUS_LOG(LOG_VERB, "[message] janus log\n");
g_print("message: g_print\n");
int send_to_clients=0;
json_t *root=load_json((const char*)vdata,vlen);//ll
if(root){
int abi=janus_process_incoming_request(c, root);
//JANUS_LOG(LOG_VERB, "janus process incoming request %d",abi);
g_print("g_print: janus process incoming request %d\n",abi);
send_to_clients=1;
}else{kore_log(LOG_INFO,"no json root in ws message");}
if(send_to_clients==0)kore_websocket_send(c, op, vdata, vlen);
if(root)json_decref(root);
}
int janus_task(struct kore_task* t){
//gint Janusmain(int argc, char *argv[])
Janusmain(1, (char*[3]){"-Nii","1", "1"});
//kore_log(LOG_INFO,"*** bye from janus task ***");
printf("*** BYE ***\n");
return (KORE_RESULT_OK);	
}
void data_available(struct kore_task*t){
size_t len;
u_int8_t buf[BUFSIZ];
if(kore_task_finished(t)){
kore_log(LOG_INFO, "task finished");
return;	
}
len=kore_task_channel_read(t, buf, sizeof(buf));
if(len>sizeof buf){kore_log(LOG_INFO,"len greater than buffer");}
kore_log(LOG_INFO,"available data is %d", len);	
}
