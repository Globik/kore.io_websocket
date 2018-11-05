#include "channel.h"
#include <sys/queue.h>
#include <time.h>
#include <unistd.h>
#include <kore/kore.h>


#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"


struct postman{
int a;
struct soup*soupi;
struct kore_timer * timer;
LIST_ENTRY (postman) rlist;	
};
LIST_HEAD(, postman) letters;
	
void channel_send(struct channel*, char*,struct soup*);
void channel_close(struct channel*);
static int pending_close(void);

static void invoke_for_dummy(char*);
static void soup_set_error(struct soup *, const char*);
static void soup_handle(struct soup*);
static void soup_release(struct soup*);

static void on_timeout(void*, u_int64_t);

struct channel *channel_new(){
LIST_INIT(&letters);
struct channel*ch = NULL;
ch=malloc(sizeof(struct channel));
if(ch==NULL)return NULL;
ch->ee=NULL;
ch->request=channel_send;
ch->close=channel_close;
return ch;	
}

static void on_timeout(void* arg, u_int64_t now){
	if(arg==NULL){printf(red "arg is NULL in on_timeout" rst); return;}
	//printf(yellow "timeout!\n" rst);
	struct soup *s = (struct soup*)arg;
	soup_set_error(s, "***TIMEOUT!***");
	s->cb(s, s->arg);
}
void channel_close(struct channel* ch){
	//printf("channel_close()\n");
	int a=pending_close();
	printf("is it OK pending_close? %d\n", a);//0 is OK
	if(ch->ee !=NULL)ch->ee=NULL;
	free(ch);
}

void channel_send(struct channel*ch, char*str, struct soup *soupi){
printf("channel_send occured\n");
if(soupi==NULL){printf(red "soupi is NULL in channel_send()\n" rst);}
char*mumu="{\"a\":\"b\"}";
char *dubu=strdup(mumu);

struct postman *db=malloc(sizeof(struct postman));
if(db==NULL){
printf("db is null, memory fails\n");
free(dubu);
return;
}

db->a=77;
db->soupi=NULL;
//soupi->result=NULL;
soupi->state=SOUP_STATE_WAIT;
db->soupi=soupi;

db->timer=kore_timer_add(on_timeout, 13000, soupi, 1);

if(db->timer==NULL){kore_log(LOG_INFO, red "*** db->timer is NULL! in channel_send ***" rst);}
LIST_INSERT_HEAD(&letters, db, rlist);
/*
int rc=uv_callback_fire(&to_cpp,(void*)dubu, NULL);
printf("fire to_cpp: %d\n", rc);//0 is OK
if(rc !=0){
LIST_REMOVE(db, rlist);
kore_timer_remove(db->timer);
db->timer=NULL;//??
db->soupi=NULL;//??
free(db);
db=NULL;//??
soup_set_error(soupi, "data to send failed.");
soupi->cb(soupi, soupi->arg);
}
*/ 
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

printf("ANY DATA? %s [%s]\n",data, __FILE__);
for(du=LIST_FIRST(&letters); du !=NULL; du=dtmp){
dtmp=LIST_NEXT(du, rlist);

if(du->a==77){

if(du->soupi){
du->soupi->result=kore_strdup(data);	
kore_timer_remove(du->timer);
du->timer=NULL;
du->soupi->state=SOUP_STATE_RESULT;
if(du->soupi->cb !=NULL)du->soupi->cb(du->soupi, du->soupi->arg);	

LIST_REMOVE(du, rlist);
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
static int pending_close(){
printf("pending_close()\n");	
struct postman*du;
while(!LIST_EMPTY(&letters)){
du=LIST_FIRST(&letters);
LIST_REMOVE(du, rlist);

if(du->soupi){
if(du->soupi->cb !=NULL){

soup_set_error(du->soupi, "channel closed.");	
kore_timer_remove(du->timer);
du->timer=NULL;
du->soupi->cb(du->soupi, du->soupi->arg);	
}
du->soupi=NULL;
free(du);
du=NULL;

}	
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
printf("before set error in soup_set_error);\n");
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
