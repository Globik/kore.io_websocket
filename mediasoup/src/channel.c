#include "channel.h"
#include <sys/queue.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>
#include <kore/kore.h>


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


static void invoke_for_dummy(char*);
static void soup_set_error(struct soup *, const char*);
static void soup_handle(struct soup*);
static void soup_release(struct soup*);

static int id_lookup(uint32_t);

static void on_timeout(void*, u_int64_t);

struct channel * channel_new(){
LIST_INIT(&letters);
struct channel*ch = NULL;
ch=malloc(sizeof(struct channel));
if(ch==NULL)return NULL;
ch->ee=NULL;
ch->request=channel_send;
ch->close=channel_close;

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
	if(ch->ee !=NULL)ch->ee=NULL;
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

static void invoke_for_dummy(char* data){
//struct responsi resp;
//resp.ch=ch;
//resp.data="room_created";
//ch->on_ersti(ch,(void*)&resp);
//ch->on_ersti=NULL;
//ee_emit(ch->ee, erste_data, (void*)&resp);
struct postman *du = NULL; 
struct postman *dtmp;

//printf("ANY DATA? %s [%s]\n",data, __FILE__);
for(du=LIST_FIRST(&letters); du !=NULL; du=dtmp){
dtmp=LIST_NEXT(du, rlist);

if(du->id == du->soupi->id){
printf("FOUND IN INVOKE FOR DUMMY\n");
if(du->soupi){
	//printf(green "it's a du->soupi! %s\n" rst, );
du->soupi->result=kore_strdup(data);	
printf(green "it's a du->soupi! %s\n" rst, du->soupi->result);
kore_timer_remove(du->timer);
du->timer=NULL;
du->soupi->state=SOUP_STATE_RESULT;
du->soupi->id=0;
du->id=0;
if(du->soupi->cb !=NULL){
	printf(green "du->soupi->cb is HERE!!\n" rst);
	du->soupi->cb(du->soupi, du->soupi->arg);	}else{
		printf(red "du->soupi->cb null??\n" rst);
		}

LIST_REMOVE(du, rlist);
du->id = 0;
du->soupi=NULL;
free(du);
du=NULL;

}

break;
}
if(du==NULL){
printf("not found\n");
//return;
}

}
	
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
if(data==NULL){printf("DATA IS NULL! from_cpp.\n");return NULL;}
printf("ON_FROM_CPP data came: %s \n",(char*)data);

//char*s=(char*)data;
invoke_for_dummy(data);
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
