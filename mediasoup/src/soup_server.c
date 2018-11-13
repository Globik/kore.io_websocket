#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <kore/kore.h>
//#include "globikCommon.h"
#include "soup_server.h"





static void closi(struct server*);

int create_room(struct server*, struct soup*, const char*);


void md_destroy(struct server*);


int create_room(struct server * serv, struct soup* soupi, const char* options){
printf("in create_room()\n");
//ee_once(server->ch->ee, zweite_data, on_zweite_data);//on .  a ccepted with data if any, rejected
uint32_t room_id=random_u32();
json_t * jso_internal = json_object();
json_object_set_new(jso_internal, "roomId", json_integer(room_id));
soupi->in_id = room_id;
serv->ch->request(serv->ch, soupi, options, jso_internal);
return 0;
}

static void emiti(struct server*server,const char*str,void*data){
ee_emit(server->ee, str, data);	
}

void md_destroy(struct server*serv){
	if(serv==NULL){printf("serv is NULL.\n");return;}
	printf("md_destroy occured for mediasoup client.\n");
	
	if(serv->ch){
	printf("looks like serv->ch still there\n");
	serv->ch->close(serv->ch);//free channel
	serv->ch=NULL;
	}
	//if(serv->name){printf("looks like serv->name still there.\n");free(serv->name);serv->name=NULL;}
	if(serv->ee)ee_destroy(serv->ee);
	free(serv);
	serv=NULL;//?
}

struct server*server_new(){
struct server*obj=NULL;
obj=malloc(sizeof(struct server));
if(obj==NULL)return NULL;
ee_t*ee=ee_new();
if(ee==NULL){printf("ee is null\n");return NULL;}
obj->ee=ee;
obj->ch=channel_new();
if(obj->ch==NULL){
printf("obj->ch is NULL\n");	
ee_destroy(ee);
obj->ee=NULL;
return NULL;
}
//obj->ch->ee=obj->ee;
obj->emit=emiti;

obj->close=closi;
obj->create_room = create_room;
obj->destroy=md_destroy;
return obj;
}

void closi(struct server*obj){
	
printf("closi occured\n");
struct out_data data;
data.str="someone closes the server.";

obj->emit(obj,"close",(void*)&data);
}

void soup_init(struct soup*s,struct server*serv){
	//if serv == NULL return;
memset(s, 0, sizeof(*s));
s->state= SOUP_STATE_INIT;	
s->conn=serv;
s->id = 0;
s->in_id = 0;
//s->name=NULL;//?? by memset scenario causes the segfault
//s->result=NULL;//?? by memset scenario is segfault
}
void soup_bind_callback(struct soup*soupi, void (*cb)(struct soup*,void*),void*arg){
//if(s->cb !=NULL) //fatal error : already bound
soupi->cb=cb;
soupi->arg=arg;
}
int make_room(struct soup*soupi, const char* room_options){
	printf("make room occured.\n");
if(!soupi)return 0;
if(soupi->conn==NULL) return 0;	
soupi->name=kore_strdup("worker.createRoom");
//char* in_room_opt=kore_strdup(room_options);
soupi->conn->create_room(soupi->conn, soupi, room_options);
return 1;
}


