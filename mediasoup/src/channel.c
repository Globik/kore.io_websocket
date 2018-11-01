#include "channel.h"
#include <sys/queue.h>
#include <time.h>
#include <unistd.h>
struct db{
//char*name;
int a;
//on_ersti_cb cb;
//struct channel*cb;
struct soup*soupi;
LIST_ENTRY (db) rlist;	
	};
LIST_HEAD(,db) strings;
	
static void channel_send(struct channel*, char*,struct soup*/*,on_ersti_cb*/);
//static void invoke_for_dummy(struct channel*);
static void invoke_for_dummy(char*);
//static void on_erst_data(void*);
//static const char*erste_data="erste_data";
//static const char*zweite_data="zweite_data";



struct channel*channel_new(){
LIST_INIT(&strings);
struct channel*ch = NULL;
ch=malloc(sizeof(struct channel));
if(ch==NULL)return NULL;
ch->ee=NULL;
ch->request=channel_send;
//ch->on_ersti=NULL;//ersti_cb;
return ch;	
}

void channel_send(struct channel*ch, char*str, struct soup*soupi /*, on_ersti_cb ersti_cb*/){
printf("channel_send occured\n");

char*mumu="{\"a\":\"b\"}";
char *dubu=strdup(mumu);
//int rc=uv_callback_fire(&to_cpp,(void*)dubu, NULL);
//printf("fire to_cpp: %d\n", rc);


struct db*db;
db=malloc(sizeof(struct db));
if(db==NULL){printf("db is null, memory fails\n");}
//db->name=strdup(str);
//db->name=strdup("worker.createRoom");
db->a=77;
db->soupi=NULL;
//soupi->result=NULL;
db->soupi=soupi;

LIST_INSERT_HEAD(&strings,db,rlist);
int rc=uv_callback_fire(&to_cpp,(void*)dubu, NULL);
printf("fire to_cpp: %d\n", rc);


//invoke_for_dummy(ch);
}
/*
void ersti_cb(struct channel*ch, char*str){
	printf("ersti cb occured.\n");
}*/
/*
void on_erst_data(void*data){
printf("on_erst_data\n");
struct responsi *resp=(struct responsi*)data;
printf("here data: %s\n",(char*)resp->data);
// accepted, rejected, targetId
//if (accepted)
//ee_emit(resp->ch->ee, zweite_data, "accepted with data if any");
}
*/
void invoke_for_dummy(/*struct channel*ch*/char*data){
	printf("invoke_for_dummy occured.\n");
//struct responsi resp;
//resp.ch=ch;
//resp.data="room_created";
//ch->on_ersti(ch,(void*)&resp);
//ch->on_ersti=NULL;
//ee_emit(ch->ee, erste_data, (void*)&resp);
struct db *du=NULL; 
struct db*dtmp;
/*
LIST_FOREACH(du, &strings, rlist){
printf("foreach_1 : %d\n", du->a);	
//if(!strcmp(du->name,"vadik"))du->cb->on_ersti(ch,(void*)&resp);
if(du->a==77){
if(du->soupi){
printf("there is a du->soupi\n");
if(du->soupi->cb){
//du->soupi->result=data;	// crash
printf("making callback for work it out.\n");du->soupi->cb(du->soupi, du->soupi->arg);	}
}
}
}
*/
printf("ANY DATA? %s [%s]\n",data,__FILE__);
for(du=LIST_FIRST(&strings); du !=NULL; du=dtmp){
	dtmp=LIST_NEXT(du,rlist);
	printf("within LIST_FIRST\n");
	//du->a=88;
	//free(du->name);
	//du->name=strdup("suchara");
	
	
if(du->a==77){
if(du->soupi){
printf("there is a du->soupi\n");
if(du->soupi->cb){
du->soupi->result=strdup(data);	
if(du->soupi->arg)printf("ok for du->soupi->arg\n");
if(du->soupi->cb)printf("ok for du->soupi->cb\n");
printf("making callback for work it out.\n");du->soupi->cb(du->soupi, du->soupi->arg);	}
}
}

	
	}
	
	/*
LIST_FOREACH(du,&strings,rlist){
printf("foreach_2  : %d\n", du->a);	
}

*/


while(!LIST_EMPTY(&strings)){
	printf("within list_empty for &strings\n");
du=LIST_FIRST(&strings);
LIST_REMOVE(du, rlist);
//if(du->name){printf("check du->name: %s\n",du->name);free(du->name);}
//du->soupi=NULL;
free(du);	
}
}

void * on_from_cpp(uv_callback_t*handle, void*data){
if(data==NULL){printf("DATA IS NULL!\n");return NULL;}
printf("ON_FROM_CPP data came: %s \n",(char*)data);

//char*s=(char*)data;
invoke_for_dummy(data);
free(data);
return "a";
}
