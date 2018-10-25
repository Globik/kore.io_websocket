#include "channel.h"
#include <sys/queue.h>
#include <time.h>
#include <unistd.h>
struct db{
char*name;
int a;
//on_ersti_cb cb;
struct channel*cb;
LIST_ENTRY (db) rlist;	
	};
LIST_HEAD(,db) strings;
	
static void channel_send(struct channel*, char*,on_ersti_cb);
static void invoke_for_dummy(struct channel*);
static void on_erst_data(void*);
static const char*erste_data="erste_data";
static const char*zweite_data="zweite_data";

unsigned long time_ms(void){
struct timespec tp;
clock_gettime(CLOCK_MONOTONIC, &tp);
return (tp.tv_sec*1000+tp.tv_nsec/1000000);	
}



struct channel*channel_new(){
	LIST_INIT(&strings);
struct channel*ch=NULL;
ch=malloc(sizeof(struct channel));
if(ch==NULL)return NULL;
ch->ee=NULL;
ch->request=channel_send;
ch->on_ersti=NULL;//ersti_cb;
return ch;	
}

void channel_send(struct channel*ch, char*str,on_ersti_cb ersti_cb){
printf("channel_send occured\n");
unsigned long start=time_ms();
long timeout=3000;
//ee_once(ch->ee, erste_data, on_erst_data);
ch->on_ersti=ersti_cb;
//method to store - str or number code
// if str==create_room code 1
// or if code==1 "create_room"

struct db*db;
db=malloc(sizeof(struct db));
if(db==NULL){printf("db is null\n");}
db->name=strdup("vadik");
db->a=77;
db->cb=NULL;//ersti_cb;
db->cb=ch;
LIST_INSERT_HEAD(&strings,db,rlist);
//sleep(5);
if((time_ms()-start) > timeout){printf("TIMEOUT_FAILED\n");}

invoke_for_dummy(ch);
}
/*
void ersti_cb(struct channel*ch, char*str){
	printf("ersti cb occured.\n");
}*/
void on_erst_data(void*data){
printf("on_erst_data\n");
struct responsi *resp=(struct responsi*)data;
printf("here data: %s\n",(char*)resp->data);
// accepted, rejected, targetId
//if (accepted)
//ee_emit(resp->ch->ee, zweite_data, "accepted with data if any");
}
void invoke_for_dummy(struct channel*ch){
struct responsi resp;
resp.ch=ch;
resp.data="room_created";
ch->on_ersti(ch,(void*)&resp);
//ch->on_ersti=NULL;
//ee_emit(ch->ee, erste_data, (void*)&resp);
struct db *du=NULL; struct db*dtmp;
LIST_FOREACH(du,&strings,rlist){
printf("foreach_1 %s : %d\n",du->name, du->a);	
if(!strcmp(du->name,"vadik"))du->cb->on_ersti(ch,(void*)&resp);
}
for(du=LIST_FIRST(&strings); du !=NULL; du=dtmp){
	dtmp=LIST_NEXT(du,rlist);
	du->a=88;
	free(du->name);
	du->name=strdup("suchara");
	}
LIST_FOREACH(du,&strings,rlist){
printf("foreach_2 %s : %d\n",du->name, du->a);	
}




while(!LIST_EMPTY(&strings)){
du=LIST_FIRST(&strings);
LIST_REMOVE(du,rlist);
if(du->name){printf("check du->name: %s\n",du->name);free(du->name);}
free(du);	
}
}
