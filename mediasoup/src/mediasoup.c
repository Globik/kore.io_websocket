#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//#include <memory.h>
#include <signal.h>

#include <unistd.h>// getpid(), usleep()

//#include <stdio.h>
#include <inttypes.h>
//#include <stdlib.h>
#include <stdint.h>


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
#include "Utils.hpp"
#include "Channel/UnixStreamSocket.hpp"
#include "Room.hpp"

#include "soup_server.h"

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"

int Putin=0;
int need_exit=0;


int init(int);
int page(struct http_request*);
int page2(struct http_request*);
int libuv_task(struct kore_task*);

void pipe_data_available(struct kore_task*);


void * on_from_cpp(uv_callback_t*,void*);

void we_can_work_it_out(struct soup*,void*);

void han(void);

void tick(void*,u_int64_t);

void m_init(void);
void m_exit(void);
void m_destroy(void);
void libuv_task_init(void);

void im_down(void);

int page_ws_connect(struct http_request*);
void websocket_connect(struct connection*);
void websocket_disconnect(struct connection*);
void websocket_message(struct connection*, u_int8_t,void*,size_t);
json_t *load_json(const char*,size_t);
json_t *load_json_str(const char*);

uint32_t random_u32(void);


struct kore_task pipe_task;

struct server*md_server=NULL;

struct pizda{
char*msg;
};

ee_t* ev=NULL;

uv_callback_t stop_worker, to_cpp;

const char*room_create_str="{\"id\":3444444333,\"method\":\"worker.createRoom\",\"internal\":{\"roomId\":35,\"sister\":\"sister_1\"},\"data\":{\"a\":1}}";

void libuv_task_init(){
//if(worker->id !=1) return (KORE_RESULT_OK); //if  cpu workers greater than 1 just comment it out for a dedicated task
//struct kore_timer*tim=NULL;
kore_task_create(&pipe_task,libuv_task);
kore_task_bind_callback(&pipe_task, pipe_data_available);
kore_task_run(&pipe_task, 0);
//tim=kore_timer_add(tick, 2000, NULL, 1);
//tim=kore_timer_add(tick,10000,NULL,1);
}

void im_down(){
kore_log(LOG_INFO,"im_down()");
need_exit=1;	
if(need_exit){
if(Putin==1){return;}
if(md_server==NULL){printf(red "md_server is NULL, returning\n" rst);return;}
soup_shutdown();
//usleep(900000);
//usleep(9000);	
usleep(5000);

//just in case
if(md_server !=NULL){m_destroy();md_server->destroy(md_server);md_server=NULL;}
}
}

void on_string(void*);

void on_string(void*arg){
printf(red "on_string occured! %s\n" rst,(char*)arg);	
	}
void han(){
kore_log(LOG_INFO, yellow "at_exit()" rst);
//im_down();
if(ev !=NULL)ee_destroy(ev);
}


uint32_t random_u32(void);
uint32_t rand32(uint32_t,uint32_t);
uint32_t rand32(uint32_t begin,uint32_t end){
uint32_t range=(end-begin)+1;
	uint32_t limit=((uint64_t)RAND_MAX+1)-(((uint64_t)RAND_MAX+1)%range);
	uint32_t randVal=rand();
	while(randVal >=limit) randVal=rand();
	return ((randVal%range)+begin);
}
uint32_t random_u32(void){
	uint32_t a=rand32(10000000,99999999);
	//printf("a: %"PRIu32"\n",a);
return (a);
}


void kore_parent_teardown(){
kore_log(LOG_INFO, red "kore_parent_teardown()" rst);	
}
const char*room_options="{\"mediaCodecs\":[{\"kind\":\"audio\",\"name\":\"audio/opus\",\"clockRate\":48000,\"payloadType\":100,\"numChannels\":2},{\"kind\":\"audio\",\"name\":\"audio/PCMU\",\"payloadType\":0,\"clockRate\":8000},{\"kind\":\"video\",\"name\":\"video/vp8\",\"payloadType\":110,\"clockRate\":90000},{\"kind\":\"video\",\"name\":\"video/h264\",\"clockRate\":90000,\"payloadType\":112,\"parameters\":{\"packetizationMode\":1}},{\"kind\":\"depth\",\"name\":\"video/vp8\",\"payloadType\":120,\"clockRate\": 90000}]}";


void kore_worker_configure(){
kore_log(LOG_NOTICE,red "worker configure" rst);
libuv_task_init();
atexit(han);
uint32_t a=random_u32();
uint32_t ba=random_u32();
kore_log(LOG_INFO, red "random uint32_t : %"PRIu32"" rst, a);
char stri[9];
snprintf(stri, sizeof stri,"%" PRIu32, a);
kore_log(LOG_INFO, red "stri: %s" rst, stri);

ev=ee_new();
ee_once(ev, stri, on_string);
//ee_emit(server->ee, str, data);	
//14289383
ee_emit(ev, "14289383","hi_string!");

json_t*reply=json_object();
json_object_set_new(reply,"method",json_string("worker.createRoom"));
json_object_set_new(reply,"id",json_integer(a));
//internal,data
json_t*js_internal=json_object();
json_object_set_new(js_internal,"roomId", json_integer(ba));
json_object_set_new(reply,"internal", js_internal);
json_t*js_data = load_json_str(room_options);
if(js_data==NULL){printf(red "js_data is NULL!\n" rst);}
json_object_set_new(reply,"data", js_data);
char*suki=json_dumps(reply,0);
json_decref(js_data);
json_decref(js_internal);
json_decref(reply);
printf("%s\n",suki);
free(suki);
uint32_t bugi=get_random32u();
printf("bugi=> %"PRIu32"\n", bugi);
//10158188
}

void kore_worker_teardown(){
kore_log(LOG_INFO, red "kore_worker_teardown occured." rst);
im_down();	
}

void tick(void*unused,u_int64_t now){
	kore_log(LOG_INFO, green " a tick occured." rst);
	if(Putin==0){
	soup_shutdown();Putin=1;
	}
	}
int init(int state){
if(state==KORE_MODULE_UNLOAD) return (KORE_RESULT_ERROR);
return (KORE_RESULT_OK);
}
int page(struct http_request*req){
http_response_header(req,"content-type","text/html");
http_response(req,200,asset_frontend_html,asset_len_frontend_html);
kore_log(LOG_NOTICE,"http request should be sent");
return (KORE_RESULT_OK);
}
int page2(struct http_request*req){
http_response_header(req,"content-type","text/html");
http_response(req,200,asset_front2_html,asset_len_front2_html);
kore_log(LOG_NOTICE,"front2.html");
return (KORE_RESULT_OK);
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
soup_bind_callback(soupi, we_can_work_it_out, c);
c->hdlr_extra=soupi;
int ra=make_room(soupi,"make_room");
printf("make room: %d\n", ra);

}

void we_can_work_it_out(struct soup *soupi,void *arg){
kore_log(LOG_INFO,"we_can_work_it_out() occured.");
kore_log(LOG_INFO, green "mem of soupi: %p" rst, (void*)soupi);
kore_log(LOG_INFO, yellow "mem of arg %p" rst, arg);
kore_log(LOG_INFO, green "soupi->state: %d" rst, soupi->state);
//if(soupi->result){kore_log(LOG_INFO, green "data: %s" rst, soupi->result);}
//if(soupi->name){kore_log(LOG_INFO, yellow "name: %s" rst, soupi->name);}
struct connection *c=arg;

switch(soupi->state){
case SOUP_STATE_INIT:
case SOUP_STATE_WAIT:
kore_log(LOG_INFO, green "SOUP_STATE_INIT or wait" rst);
break;
case SOUP_STATE_RESULT:
if(soupi->result !=NULL){
kore_log(LOG_INFO, green "SOUP_STATE_RESULT\n" rst);
if(soupi->name){kore_log(LOG_INFO, green "name: %s" rst, soupi->name);}
kore_websocket_send(c,1,soupi->result,strlen(soupi->result));
//free(soupi->result);
//soupi->result=NULL;
}
soupi->state=SOUP_STATE_DONE;
break;
case SOUP_STATE_DONE:
kore_log(LOG_INFO, green "SOUP_STATE_DONE" rst);
break;
case SOUP_STATE_COMPLETE:
kore_log(LOG_INFO, green "SOUP_STATE_COMPLETE" rst);
break;
case SOUP_STATE_ERROR:
kore_log(LOG_INFO, red "SOUP_STATE_ERROR" rst);
if(soupi->error !=NULL){
kore_log(LOG_INFO, red "it's soupi->error: %s" rst, soupi->error);
kore_websocket_send(c,1,soupi->error,strlen(soupi->error));
//kore_free(soupi->error);
//soupi->error=NULL;
	
}
soupi->state=SOUP_STATE_DONE;//?
break;
default:
kore_log(LOG_INFO, green "SOUP_STATE_DEFAULT" rst);
//soup_continue(soupi);
}

/*

if(soupi->result !=NULL){
printf(green "free soupi->result\n" rst);
kore_websocket_send(c,1,soupi->result,strlen(soupi->result));
free(soupi->result);
soupi->result=NULL;
}
*/
/* 
if(soupi->name){
printf(green "free soupi->name\n" rst);
free(soupi->name);
soupi->name=NULL;
}
 */
/*
if(soupi->error){
kore_log(LOG_INFO, red "it's soupi->error %s" rst, soupi->error);
kore_websocket_send(c,1,soupi->error,strlen(soupi->error));
kore_free(soupi->error);
soupi->error=NULL;	
}
*/ 
	printf("SOME AFTER DEFAULT MUST BE??\n");
	soup_continue(soupi);
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
json_object_set_new(repli,"id",json_integer(344444433));
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
kore_log(LOG_INFO,"rc: %d",rc);




send_to_clients=1;
}else if(!strcmp(type_str,"delete_room")){
kore_log(LOG_INFO,"type 'delete_room'");
char*d="{\"id\":344444433,\"method\":\"room.close\",\"internal\":{\"roomId\":35,\"sister\":\"sister_1\"},\"data\":{\"a\":1}}\0xe0";

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
kore_log(LOG_INFO,red "websocket disconnected: %p" rst,c);
if(c->hdlr_extra !=NULL)
	{
		//free soupi
		printf(yellow "free c->hdlr_extra\n" rst);
		//c->hdlr_extra->conn=NULL;
		kore_free(c->hdlr_extra);
	c->hdlr_extra=NULL;
	//free(c);
}
}

int libuv_task(struct kore_task*t){
kore_log(LOG_NOTICE,"A task created");
//atexit(han);
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
set_soup_loop(chl);// it's a Loop loop(channel)
m_destroy();
//kore_task_channel_write(t,"mama\0",5);
//kore_log(LOG_NOTICE,"*** MMM Bye. *******\n");
if(md_server){md_server->destroy(md_server);md_server=NULL;}
//m_exit();

kore_log(LOG_NOTICE,"Bye. *******\n");

if(md_server==NULL){
//if(need_exit)exit(0);
}
return (KORE_RESULT_OK);
}
void pipe_data_available(struct kore_task*t){
	kore_log(LOG_INFO, yellow "pipe_data_available" rst);
if(kore_task_finished(t)){
	kore_log(LOG_INFO, green "a task is finished." rst);
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
	utils_crypto_class_init();

	rtc_room_classinit();
	//rtc_room_classini
}
void m_exit(){
//usleep(100000);
//kore_log(LOG_INFO,"***SUCCESS: And exit with success status.");
//_exit(0);
}
void m_destroy(){
kore_log(LOG_INFO,red "Destroy m_destroy()." rst);
//usleep(10000);
//usleep(10);
utils_crypto_class_destroy();
class_destroy();
//if(md_server){md_server->destroy(md_server);md_server=NULL;}
//usleep(1000000);
//usleep(1);
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

/*
 * 
 * 
 * Correct
 * 
 * Signal INT received, exiting.
[parent]: server shutting down
Loop::Close() entered.
[parent]: waiting for workers to drain and shutdown
Closing signalsHandler.
signal destroy
on_walk unixstream socket
on_walk unixstream socket
on_walk unixstream socket
on_walk unixstream socket
Look ma, ~UnixStreamSocket() destructor!
on close sig handler
on close sig handler
Good bye, libuv's loop!
The loop should be ending now!
Look ma, ~Loop() destructor.
[wrk 0]: Destroy m_destroy().
Look ma, loop is destroyd.
[wrk 0]: A message came: mama
[wrk 0]: *** MMM Bye. *******

md_destroy occured for mediasoup client.
looks like serv->ch still there
looks like serv->name still there.
[wrk 0]: A message came: papa
[wrk 0]: a task is finished.
[parent]: worker 0 (8690)-> status 0
[parent]: goodbye
* 
* UNcorect
* 
*  Signal INT received, exiting.
Loop::Close() entered.
Closing signalsHandler.
signal destroy
[parent]: server shutting down
[parent]: waiting for workers to drain and shutdown
[parent]: worker 0 (8766)-> status 2
[parent]: goodbye


 */
