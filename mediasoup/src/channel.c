#include "channel.h"
#include <sys/queue.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>
#include <kore/kore.h>
//#include <ee.h>

#include "globikCommon.h"


#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"


	

struct postman{
uint32_t id;
struct soup* soupi;
struct kore_timer * timer;
LIST_ENTRY (postman) rlist;	
};
LIST_HEAD(, postman) letters;


	
void channel_send(struct channel *, struct soup*, const char*, json_t*);
void channel_close(struct channel*);
static int pending_close(void);


static void invoke_for_dummy(json_t * data);
static void soup_set_error(struct soup *, const char*);
static void soup_handle(struct soup*);
static void soup_release(struct soup*);

static int id_lookup(uint32_t);

static void on_timeout(void*, u_int64_t);

struct notify_s{
struct channel *ch;	
void(*angabe)(struct notify_s, char*, char*);
}notify;

void angabe(struct notify_s, char*, char*);
void angabe(struct notify_s nota, char* s, char* msg_data){
printf("angabe\n");	
printf("msg_data: %s\n", msg_data);
printf("target id: %s\n", s);
//ee_emit(ee, stri_uint, "message");	
//ee_emit(nota->ch->ee, s, msg_data);
ee_emit(nota.ch->ee, s, msg_data);
}
struct channel * channel_new(){
LIST_INIT(&letters);
struct channel*ch = NULL;
ch=malloc(sizeof(struct channel));
if(ch==NULL)return NULL;
ch->ee=ee_new();
if(ch->ee==NULL){
printf(red "ee is NULL\n" rst);
free(ch);
ch=NULL;
return NULL;
}
ch->request=channel_send;
ch->close=channel_close;
//ch->datei=get_data;
//struct notify_s notify;
notify.ch=ch;
notify.angabe=angabe;
return ch;	
}

static void on_timeout(void* const arg_data, u_int64_t now){
	//if(arg_data == NULL){printf(red "arg_data is NULL in on_timeout" rst); return;}
	printf(yellow "timeout!\n" rst);
	
	struct soup *s = arg_data;
	soup_set_error(s, "***TIMEOUT!***");
	s->cb(s, s->arg);	
struct postman *du = NULL; 
struct postman *dtmp;

for(du=LIST_FIRST(&letters); du !=NULL; du=dtmp){
dtmp=LIST_NEXT(du, rlist);
printf("du->id: %"PRIu32"\n",du->id);
if(du->id == s->id)break;
}

if(du == NULL){
printf(red "du is NULL\n");
return;
}else{printf(red "du is NOT NULL\n" rst);}

if(du->timer !=NULL){
printf(yellow "du->timer is NOT NULL\n" rst);
du->timer=NULL;
}
du->id = 0;
LIST_REMOVE(du, rlist);
//printf(green "after LIST_REMOVE\n" rst);
du->soupi=NULL;
free(du);
du=NULL;
s->id = 0;
}

void channel_close(struct channel* ch){
	//printf("channel_close()\n");
	int a=pending_close();
	printf("is it OK pending_close? %d\n", a);//0 is OK
	if(ch->ee !=NULL){
	ee_destroy(ch->ee);
	ch->ee=NULL;
	}
	free(ch);
}

void channel_send(struct channel* ch, struct soup *soupi, const char* options, json_t* jso_internal){
printf("channel_send occured\n");

if(soupi==NULL){printf(red "why soupi is NULL in channel_send()?\n" rst);}
printf("soupi->id: %"PRIu32"\n", soupi->id);
struct postman *db=malloc(sizeof(struct postman));
if(db == NULL){
printf(red "struct postman memory fails\n" rst);
return;
}


uint32_t req_id =0;
if(soupi->id==0){
while(req_id == 0){
req_id = random_u32();
if(id_lookup(req_id) !=0){printf(red "id_lookup(req_id)==0!\n" rst);req_id = 0;}
printf(green "id_lookup(req_id) = -1\n" rst);	
}
}else{printf(red "why soupi->id is not 0??\n" rst);}
//random_u32();
//todo check if exists

if(soupi->name==NULL){printf(red "why soupi->name is NULL?\n");}
json_t* jso_request = json_object();
json_object_set_new(jso_request,"method", json_string(soupi->name));
json_object_set_new(jso_request, "id", json_integer(req_id));
//internal,data
//json_t* jso_internal = json_object();
//json_object_set_new(jso_internal, "roomId", json_integer(room_id));
json_object_set_new(jso_request,"internal", jso_internal);
json_t* jso_data = load_json_str(options);
if(!jso_data){printf(red "why json_data is NULL!?\n" rst);}
json_object_set_new(jso_request, "data", jso_data);
char* req_string = json_dumps(jso_request, 0);
json_decref(jso_data);
json_decref(jso_internal);
json_decref(jso_request);
printf("req_string: %s\n", req_string);

soupi->id = req_id;
db->id = req_id;
//db->soupi=NULL;
//soupi->result=NULL;
soupi->state=SOUP_STATE_WAIT;
db->soupi=soupi;

db->timer=kore_timer_add(on_timeout, 16000, soupi, 1);

if(db->timer==NULL){kore_log(LOG_INFO, red "*** why db->timer is NULL! in channel_send? ***" rst);}
LIST_INSERT_HEAD(&letters, db, rlist);
//free(req_string);
//char*suki=strdup("{\"id\":45444444}");
int rc=uv_callback_fire(&to_cpp,(void*)req_string, NULL);
printf("fire to_cpp: %d\n", rc);//0 is OK
if(rc !=0){
LIST_REMOVE(db, rlist);
kore_timer_remove(db->timer);
db->timer=NULL;//??
db->soupi=NULL;//??
db->id = 0;
free(db);
db=NULL;//??
soup_set_error(soupi, "data to send failed.");
soupi->cb(soupi, soupi->arg);
soupi->id = 0;
}

}
uint32_t js_get_uint32(const json_t * const root, const char*key);
int if_exist_key(const json_t * const root, const char*key);

int if_exist_key(const json_t * const root, const char*key){
const json_t * const k = json_object_get(root, key);
if(k){return 1;}
return 0;	
}


uint32_t js_get_uint32(const json_t * const root, const char*key){
const json_t * const res_id = json_object_get(root, key);
if(res_id && json_is_integer(res_id)){
uint32_t id = json_integer_value(res_id);
return id;
}
return 0;
}

static void invoke_for_dummy(json_t * data){
//struct responsi resp;
//resp.ch=ch;
//resp.data="room_created";
//ch->on_ersti(ch,(void*)&resp);
//ch->on_ersti=NULL;
//ee_emit(ch->ee, erste_data, (void*)&resp);
json_t* root=data;
//if(!json_is_object(root))bla bla error
//uint32_t msg_target_id = 0;
if(if_exist_key(root, "id")==1){
printf("key 'id' exists\n");
uint32_t res_id=js_get_uint32(root, "id");
if(res_id==0){printf(red "why id is 0?\n" rst);}

struct postman *du = NULL; 
struct postman *dtmp;
int found_ok=0;
//printf("ANY DATA? %s [%s]\n",data, __FILE__);
for(du=LIST_FIRST(&letters); du !=NULL; du=dtmp){
dtmp=LIST_NEXT(du, rlist);

if(du->id == res_id){
printf("FOUND IN INVOKE FOR DUMMY\n");
if(du->soupi){
	//printf(green "it's a du->soupi! %s\n" rst, );
	
if(if_exist_key(root, "accepted")==1){
if(du->soupi->result !=NULL)kore_free(du->soupi->result);
json_t* js_data = json_object_get(root, "data");
char*tmp_data=NULL;
char* txt_js_data = json_dumps(js_data, 0);
if(txt_js_data) {
tmp_data=kore_strdup(txt_js_data);
free(txt_js_data);
}else{
tmp_data=kore_strdup("some data if any");	
}
du->soupi->result=tmp_data;//kore_strdup("some_data");	
du->soupi->state=SOUP_STATE_RESULT;
if(du->soupi->cb !=NULL){
printf(green "du->soupi->cb is HERE!!\n" rst);
du->soupi->cb(du->soupi, du->soupi->arg);	}else{
printf(red "du->soupi->cb null??\n" rst);
kore_free(tmp_data);//??
tmp_data=NULL;
}
}else if(if_exist_key(root, "rejected")==1){
du->soupi->state=SOUP_STATE_ERROR;
soup_set_error(du->soupi, "some_error");
if(du->soupi->cb !=NULL){
printf(green "du->soupi->cb is HERE!!\n" rst);
du->soupi->cb(du->soupi, du->soupi->arg);	}else{
printf(red "du->soupi->cb null??\n" rst);
}
	
	
}


kore_timer_remove(du->timer);
du->timer=NULL;
//du->soupi->state=SOUP_STATE_RESULT;
du->soupi->id=0;
du->id=0;


LIST_REMOVE(du, rlist);
du->id = 0;
du->soupi=NULL;
free(du);
du=NULL;

}
found_ok=1;
break;
}


}//for loop
if(found_ok==0){
printf(red "not found\n" rst); 
if(root)json_decref(root);
return;
}
//if msg.id
}else if((if_exist_key(root, "targetId")==1) && (if_exist_key(root, "event")==1)){
printf(yellow "here must be event emitter, got targetId && event\n" rst);
uint32_t target_id = js_get_uint32(root, "targetId");
printf("targetId: %"PRIu32"\n", target_id);
char stri_uint[9];
snprintf(stri_uint, sizeof stri_uint,"%" PRIu32, target_id);
kore_log(LOG_INFO, red "target_id str: %s" rst, stri_uint);
//ee_emit(ee, stri_uint, "message");	
notify.angabe(notify, stri_uint, "hallo world!");
}else{
printf(red "received message is not a response nor a notification\n" rst);
}
if(root)json_decref(root);
}


static int id_lookup(uint32_t z_id){
struct postman *p = NULL;
LIST_FOREACH(p, &letters, rlist){
if(p->id == z_id) return -1;
}
return 0;	
}



static int pending_close(){
printf("pending_close()\n");	
struct postman*du;
while(!LIST_EMPTY(&letters)){
du=LIST_FIRST(&letters);
LIST_REMOVE(du, rlist);

if(du->timer==NULL){
printf("du->timer is NULL\n");}else{printf("du->timet is not null\n");
kore_timer_remove(du->timer);
}
du->timer=NULL;
du->id = 0;
du->soupi=NULL;
free(du);
du=NULL;
}
return 0;
} 

void * on_from_cpp(uv_callback_t *handle, void*data){
if(data==NULL){
printf(red "*** DATA IS NULL! FROM_CPP!!! ***.\n" rst);
//uv_stop(((uv_handle_t*)handle)->loop);
//uv_walk(deplibuv::getloop(), on_walk, NULL);
//uv_close(handle);
 return NULL;
 
 }

json_t * cpp_json = load_json_str(data);
if(cpp_json){
printf("cpp_json is ok\n");

invoke_for_dummy(cpp_json);


//json_decref(cpp_json);	
}else{
printf("not json data came\n");	
printf("ON_FROM_CPP data came: %s \n",(char*)data);
}

//char*s=(char*)data;
//invoke_for_dummy(data);
free(data);
return (NULL);
}

static void soup_set_error(struct soup *s, const char *msg){
if(s->error !=NULL){kore_free(s->error);}
printf("before set error in soup_set_error);::: %s\n", msg);
s->error=kore_strdup(msg);
s->state=SOUP_STATE_ERROR;	
}

void soup_continue(struct soup* soupi){
printf("soup_continue()\n");
if(soupi->error){
printf("there is soupi->error\n");
kore_free(soupi->error);
soupi->error=NULL;	
}
if(soupi->result){
printf("there is soupi->result\n");
kore_free(soupi->result);
soupi->result=NULL;	
}
if(soupi->name){
printf("there is soupi->name\n");
kore_free(soupi->name);
soupi->name=NULL;	
}	
switch(soupi->state){
case SOUP_STATE_INIT:
case SOUP_STATE_WAIT:
break;
case SOUP_STATE_DONE:
printf("state done\n");
soup_release(soupi);
break;
case SOUP_STATE_RESULT:
case SOUP_STATE_ERROR:
soup_handle(soupi);//??
break;
default:
printf(red "UNKNOWN SOUP STATE!\n" rst);	
}
}
static void soup_handle(struct soup* s){
printf("soup_handle(), in fact complete and done\n");	
}
static void soup_release(struct soup* s){
printf("soup_release()\n");	
//s->state = SOUP_STATE_COMPLETE;
//if(s->cb !=NULL) s->cb(s, s->arg);
}
