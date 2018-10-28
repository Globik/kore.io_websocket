#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//#include <memory.h>
#include <signal.h>

#include <unistd.h>// getpid(), usleep()
#include <sys/types.h>
#include <pwd.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>


#include "assets.h"
#include "uv_callback.h"
#include <uv.h>
#include <jansson.h>

#include "deplibuv.hpp"
#include "Loop.hpp"
#include "Channel/UnixStreamSocket.hpp"
#include "Room.hpp"

#include "soup_server.h"

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"


int my_test = 1;
int init(int);
int page(struct http_request*);
int page2(struct http_request*);
int libuv_task(struct kore_task*);
void pipe_data_available(struct kore_task*);

void * stop_worker_cb(uv_callback_t*,void*);
//void * on_bus(uv_callback_t*, void*);
void * on_from_cpp(uv_callback_t*,void*);

void connection_del(struct connection*);
void we_can_work_it_out(struct soup*,void*);

void han(void);

void m_init(void);
void m_exit(void);
void m_destroy(void);

int page_ws_connect(struct http_request*);
void websocket_connect(struct connection*);
void websocket_disconnect(struct connection*);
void websocket_message(struct connection*, u_int8_t,void*,size_t);
json_t *load_json(const char*,size_t);
json_t *load_json_str(const char*);


struct kore_task pipe_task;

struct server*md_server=NULL;

struct pizda{
char*msg;
};

uv_callback_t stop_worker, to_cpp;
const char*room_create_str="{\"id\":3444444333,\"method\":\"worker.createRoom\",\"internal\":{\"roomId\":35,\"sister\":\"sister_1\"},\"data\":{\"a\":1}}";

void han(){
//int rc=uv_callback_fire(&stop_worker,NULL,NULL);
//kore_log(LOG_NOTICE,"rc stop_worker fire %d",rc);
//usleep(20000);
kore_log(LOG_NOTICE,"at exit han()");
}
void kore_worker_configure(){
kore_log(LOG_NOTICE,"worker configure");
atexit(han);
}
int init(int state){
if(state==KORE_MODULE_UNLOAD) return (KORE_RESULT_ERROR);
	//if(worker->id !=1) return (KORE_RESULT_OK); //if  cpu workers great than 1 comment it out for a dedicated task
kore_task_create(&pipe_task,libuv_task);
kore_task_bind_callback(&pipe_task, pipe_data_available);
kore_task_run(&pipe_task);

return (KORE_RESULT_OK);
}
int page(struct http_request*req){
http_response_header(req,"content-type","text/html");
// Fire callback to trans a message to libuv.cpp class and check if it blocks kore workflow
// as a "request" mechanism from the kore's world to the libuv.cpp class.
//int rc=uv_callback_fire(&to_cpp,(void*)"CPP IS OK?", NULL);
// if 0 then OK
//kore_log(LOG_NOTICE,"rc to_cpp fire: %d\n",rc);
http_response(req,200,asset_frontend_html,asset_len_frontend_html);
kore_log(LOG_NOTICE,"http request should be sent");
return (KORE_RESULT_OK);
}
int page2(struct http_request*req){
// open front2[html] in a new browser's tab to check if the uv_loop blocks the frontend_html
http_response_header(req,"content-type","text/html");
http_response(req,200,asset_front2_html,asset_len_front2_html);
kore_log(LOG_NOTICE,"front2.html");
return (KORE_RESULT_OK);
}
void * stop_worker_cb(uv_callback_t*handle,void*arg){
kore_log(LOG_NOTICE,"\n on stop worker cb\n");
uv_stop(((uv_handle_t*)handle)->loop);
// s
return NULL;
}
/*
void*on_from_cpp(uv_callback_t*handle,void*data){
if(data==NULL){kore_log(LOG_INFO,"DATA IS NULL!");return NULL;}

kore_log(LOG_INFO,"ON_FROM_CPP data came: %s",(char*)data);
char*s=(char*)data;

	
json_t * duri=load_json_str(data);
	if(duri){
	kore_log(LOG_INFO,"json root is ok");
	json_decref(duri);
	}else{
	kore_log(LOG_INFO,"json root is not ok.");
	}
	

json_auto_t*reply=json_object();
json_object_set_new(reply,"type",json_string("on_result"));
json_object_set_new(reply,"msg",json_string(s));
size_t size=json_dumpb(reply,NULL,0,0);
if(size==0) {return NULL;}
char *buf=alloca(size);
size=json_dumpb(reply,buf,size,0);
kore_log(LOG_INFO,"SIZE: %d",size);

//kore_log(LOG_INFO,"BUFFER JSON: %s",buf);
kore_log(LOG_INFO,"Here must be kore_websocket_send();");
kore_websocket_broadcast(NULL,WEBSOCKET_OP_TEXT,buf,size,WEBSOCKET_BROADCAST_GLOBAL);

if(duri)free(data);
data=NULL;

//return NULL;
return "saka";
}
*/
int page_ws_connect(struct http_request*req){
kore_log(LOG_INFO,"path: %s, http_request: %p",req->path,req);
kore_websocket_handshake(req,"websocket_connect","websocket_message","websocket_disconnect");
return (KORE_RESULT_OK);
}

void websocket_connect(struct connection*c){
kore_log(LOG_INFO,"websocket connected: %p",c);
json_auto_t*reply=json_object();
json_object_set_new(reply,"type",json_string("ack"));
json_object_set_new(reply,"msg",json_string("hello!"));
size_t size=json_dumpb(reply,NULL,0,0);
if(size==0) return;
char *buf=alloca(size);
size=json_dumpb(reply,buf,size,0);
kore_websocket_send(c,1,buf,size);
struct soup*soupi=kore_calloc(1, sizeof(*soupi));
if(soupi==NULL){kore_log(LOG_INFO, "soupi is NULL");}
soup_init(soupi, md_server);//?
c->disconnect=connection_del;
c->state=CONN_STATE_ESTABLISHED;
soup_bind_callback(soupi, we_can_work_it_out, c);
c->hdlr_extra=soupi;
int ra=make_room(soupi,"make_room");
printf("make room: %d\n", ra);
}
void connection_del(struct connection*c){
	kore_log(LOG_INFO,"connection_del() occured.");
	if(c->hdlr_extra !=NULL)
	{
		//free soupi
		kore_free(c->hdlr_extra);
	c->hdlr_extra=NULL;
}
}
void we_can_work_it_out(struct soup *soupi,void *arg){
kore_log(LOG_INFO,"we_can_work_it_out() occured.");
	
}
void websocket_message(struct connection*c,u_int8_t op,void*data,size_t len){
if(data==NULL)return;
int send_to_clients=0;
json_auto_t*root=load_json((const char*)data,len);
if(!root)return;
json_t *type_f=json_object_get(root,"type");
const char*type_str=json_string_value(type_f);
if(!strcmp(type_str,"message")){
kore_log(LOG_INFO,"type 'message'");
}else if(!strcmp(type_str,"create_room")){
kore_log(LOG_INFO,"type 'create_room'");
	/*
	//char*s1="{\"id\":3444444333,\"method\":\"worker.createRoom\",\"internal\":{\"roomId\":35,\"sister\":\"sister_1\"},\"data\":{\"a\":1}}";
*/
json_auto_t*repli=json_object();
json_object_set_new(repli,"id",json_integer(3444444333));
json_object_set_new(repli,"method",json_string("worker.createRoom"));
	
json_t*repli_internal=json_object();
json_object_set_new(repli_internal,"roomId",json_integer(35));
json_object_set_new(repli_internal,"sister",json_string("sister_1"));
	
json_auto_t*repli_data=json_object();
json_object_set_new(repli_data,"a",json_integer(1));
	
json_object_set_new(repli,"internal",repli_internal);
json_object_set_new(repli,"data",repli_data);
size_t size=json_dumpb(repli,NULL,0,0);
	if(size==0)return;
	char*buf=alloca(size);
	size=json_dumpb(repli,buf,size,0);
	int rc;
rc=uv_callback_fire(&to_cpp,(void*)buf, NULL);
//rc=uv_callback_fire(&to_cpp,"{\"me\":\"too\"}", NULL);
//char*resp=NULL;

//rc=uv_callback_fire_sync(&to_cpp,"{\"data\":\"too\"}",(void**)&resp,10000);//no effect with result
//kore_log(LOG_INFO,"uv_callback_t &to_cpp fire %d",rc);
//kore_log(LOG_INFO,"The result is: %s",resp);










send_to_clients=1;
}else if(!strcmp(type_str,"delete_room")){
kore_log(LOG_INFO,"type 'delete_room'");
char*d="{\"id\":3444444333,\"method\":\"room.close\",\"internal\":{\"roomId\":35,\"sister\":\"sister_1\"},\"data\":{\"a\":1}}\0xe0";

int rc=uv_callback_fire(&to_cpp,(void*)d, NULL);
kore_log(LOG_INFO,"uv_callback_t &to_cpp fire %d",rc);
send_to_clients=1;
}else if(!strcmp(type_str,"room_dump")){
kore_log(LOG_INFO,"type 'room_dump'");
	// "room.createPeer" room.dump
char*d1="{\"id\":3444444333,\"method\":\"room.createPeer\",\"internal\":{\"roomId\":35,\"sister\":\"sister_1\"},\"data\":{\"a\":1}}";
int rc=uv_callback_fire(&to_cpp,(void*)d1, NULL);
kore_log(LOG_INFO,"uv_callback_t &to_cpp fire %d",rc);
send_to_clients=1;
}else{
kore_log(LOG_INFO,"Unknown type: %s",type_str);
send_to_clients=1;
}
if(send_to_clients==0)kore_websocket_send(c,op,data,len);
}
void websocket_disconnect(struct connection*c){
kore_log(LOG_INFO,"websocket disconnected: %p",c);
}

int libuv_task(struct kore_task*t){
kore_log(LOG_NOTICE,"A task created");
kore_task_channel_write(t,"mama\0",5);

class_init();
void*chl=set_channel();
m_init();
//int rc;
//char*s="{\"mama\":\"papa\"}";
	/*
char*s="{\"id\":3444444333,\"method\":\"worker.createRoom\",\"internal\":{\"roomId\":35,\"sister\":\"sister_1\"},\"data\":{\"a\":1}}";

rc=uv_callback_fire(&to_cpp,(void*)s, NULL);
kore_log(LOG_INFO,"uv_callback_t &to_cpp fire %d",rc);
char*d="{\"id\":3444444333,\"method\":\"room.close\",\"internal\":{\"roomId\":35,\"sister\":\"sister_1\"},\"data\":{\"a\":1}}";
usleep(1000);
 rc=uv_callback_fire(&to_cpp,(void*)d, NULL);
kore_log(LOG_INFO,"uv_callback_t &to_cpp fire %d",rc);
	*/
md_server=server_new();
if(md_server==NULL){kore_log(LOG_INFO,red "md_server is NULL!" rst);}
suka(chl);// it's a Loop loop(channel)
m_destroy();
kore_task_channel_write(t,"mama\0",5);
kore_log(LOG_NOTICE,"*** MMM Bye. *******\n");
if(md_server){md_server->destroy(md_server);md_server=NULL;}
kore_task_channel_write(t,"papa\0",5);
m_exit();
//usleep(100000);

//usleep(100000);
//kore_log(LOG_NOTICE,"Bye. *******\n");
return (KORE_RESULT_OK);
}
void pipe_data_available(struct kore_task*t){
if(kore_task_finished(t)){kore_log(LOG_WARNING,"a task is finished.");
	//_exit(0);
	return;
	}
   u_int8_t buf[BUFSIZ];
   size_t len=kore_task_channel_read(t,buf,sizeof(buf));
   if(len>sizeof(buf)) kore_log(LOG_WARNING,"truncated data message from task.");
   kore_log(LOG_NOTICE,"A message came: %s",buf);
   
}

void m_init()
{
	deplibuv_printversion();
	rtc_room_classinit();
	//rtc_room_classini
}
void m_exit(){
//usleep(100000);
//kore_log(LOG_INFO,"***SUCCESS: And exit with success status.");
_exit(0);
}
void m_destroy(){
kore_log(LOG_INFO,"Destroy m_destroy().");
class_destroy();
}

json_t *load_json(const char*text,size_t buflen){
json_t *root;
json_error_t error;
root=json_loadb(text,buflen,0,&error);
if(root){
return root;
}else{
kore_log(LOG_INFO,"json error on line %d: %s",error.line,error.text);
return (json_t*)0;
}
}
json_t *load_json_str(const char*text){
json_t *root;
json_error_t error;
root=json_loads(text,0,&error);
if(root){
return root;
}else{
kore_log(LOG_INFO,"json error on line %d: %s",error.line,error.text);
return (json_t*)0;
}
}
