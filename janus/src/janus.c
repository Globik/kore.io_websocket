// see assets folder index.html for videoroom plugin
#include <kore/http.h>
#include "janus.h"
#include <kore/tasks.h>
#include <jansson.h>
#include "assets.h"

int		page(struct http_request *);
int testev(struct http_request*);
int subscriber_watch(struct http_request*);
int adminwebrtc(struct http_request*);

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
usleep(500000);
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
int testev(struct http_request*req){
const char*custom;const char* custom2;
char*body;
ssize_t ret;
struct kore_buf*buf;
u_int8_t data[BUFSIZ];
if(req->method !=HTTP_METHOD_POST){
http_response(req,HTTP_STATUS_METHOD_NOT_ALLOWED,"ono",3);
return (KORE_RESULT_OK);	
}
if(http_request_header(req,"X-Requested-With",&custom)){
g_print("%s\n",custom);	
//kix
}
if(http_request_header(req,"Content-Type",&custom2)){
g_print("%s\n",custom2);	
}
buf=kore_buf_alloc(128);
for(;;){
ret=http_body_read(req,data,sizeof(data));
if(ret==-1){
kore_buf_free(buf);
g_print("err reading body\n");
http_response(req,500,"nob",3);
return (KORE_RESULT_OK);	
}	
if(ret==0)break;
kore_buf_append(buf,data,ret);
}
body=kore_buf_stringify(buf,NULL);
g_print("body: %s\n",body);
kore_buf_free(buf);
http_response(req,200,"ok\n",3);
return (KORE_RESULT_OK);
}
int page_ws_connect(struct http_request*req){
	g_print("websocket connected %s %p\n", req->path, req);
	//g_print("YES connect\n");
	kore_websocket_handshake(req,"websocket_connect", "websocket_message", "websocket_disconnect");
return (KORE_RESULT_OK);
}

int
page(struct http_request *req)
{
	//http_response(req, 200, NULL, 0);
	http_response_header(req,"content-type","text/html");
	
	//http_response(req,200, asset_index_html, asset_len_index_html);//janus.plugin.videoroom
	//http_response(req, 200, asset_echotest_html, asset_len_echotest_html);
	http_response(req, 200, asset_videoBroadcast_html, asset_len_videoBroadcast_html);// two in one
	return (KORE_RESULT_OK);
}

int subscriber_watch(struct http_request*req){
http_response_header(req,"content-type","text/html");
http_response(req,200,asset_indexWatcher_html, asset_len_indexWatcher_html);
return (KORE_RESULT_OK);	
}
int adminwebrtc(struct http_request*req){
http_response_header(req,"content-type","text/html");
http_response(req,200,asset_adminwebrtc_html, asset_len_adminwebrtc_html);
return (KORE_RESULT_OK);	
}
void websocket_connect(struct connection*c){
g_print("websocket connected %p\n",c);
if(c->hdlr_extra==NULL){
struct usi* us=kore_malloc(sizeof(*us));
if(us==NULL)return;
us->sid=0;
us->aw=0;
us->hid=0;
c->hdlr_extra=us;
}

}
void websocket_disconnect(struct connection*c){
g_print("g_print: websocket disconnected %p\n",c);
if(c->hdlr_extra !=NULL){
struct usi*us=(struct usi*)c->hdlr_extra;

if(us->sid > 0 && us->hid == 0)del_sess(us->sid);
if(us->sid > 0 && us->hid > 0){int a=del_han(us->hid, us->sid);g_print("IS OK Sid and hid deleted? %d\n", a);}
if(us)kore_free(us);
c->hdlr_extra=NULL;	
}
}
void websocket_message(struct connection*c,u_int8_t op, void* vdata, size_t vlen){
g_print("message: g_print\n");
int send_to_clients=0;
int abba_id=0;

struct usi* usa=NULL;
if(c->hdlr_extra !=NULL){
usa=(struct usi*)c->hdlr_extra;
}
json_t *root=load_json((const char*)vdata,vlen);//ll
if(root){
json_t *sasha = json_object_get(root, "adtype");
if(sasha && json_is_integer(sasha)){
abba_id = json_integer_value(sasha);
if(usa) usa->aw=abba_id;
json_decref(root);
return;
}
if(usa->aw==0){
int abi=janus_process_incoming_request(c, root);
//JANUS_LOG(LOG_VERB, "janus process incoming request %d",abi);
g_print("g_print: janus process incoming request %d\n",abi);
}else if(usa->aw==1){
g_print("*** ENTERING ADMIN REQUEST ***\n");
int ab2=janus_process_incoming_admin_request(c,root);
g_print("process admin request %d\n", ab2);	
}
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
