#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include "soup_server.h"

const char*zweite_data="zweite_data";
static void closi(struct server*);

int create_room(struct server*, int, struct soup*);
void on_zweite_data(void*);
int on_funi(void *);
void md_destroy(struct server*);

int on_funi(void*s){
printf("on_funi=> %s\n", (char*)s);
//create_room(d,
//instance for room: room_on_close, on_new_listener
return 0;	
}
/*
void on_zweite_data(void*data){
printf("on_zweite_data: %s\n",(char*)data);
//here create a room instance
on_funi("me too");//??
}
*/
/*
void ersti_cb(struct channel*ch, void*str){
printf("ersti cb occured.\n");
struct responsi *resp=(struct responsi*)str;
printf("here data: %s\n",(char*)resp->data);
//ch->on_ersti=NULL;
on_funi(resp->data);
}
*/
int create_room(struct server * serv, int a, struct soup* soupi){
printf("in create_room()\n");
//ee_once(server->ch->ee, zweite_data, on_zweite_data);//on .  a ccepted with data if any, rejected
serv->name=strdup("worker.createRoom");//??

serv->ch->request(serv->ch, "create_room", soupi);
return 0;
}

static void emiti(struct server*server,const char*str,void*data){
ee_emit(server->ee, str, data);	
}

void md_destroy(struct server*serv){
	printf("md_destroy occured for mediasoup client.\n");
	if(serv->ee)ee_destroy(serv->ee);
	if(serv->ch){printf("looks like serv->ch still there\n");free(serv->ch);serv->ch=NULL;}
	if(serv->name){printf("looks like serv->name still there.\n");free(serv->name);serv->name=NULL;}
	free(serv);
}

struct server*server_new(){
//if(ch==NULL)return NULL;
struct server*obj=NULL;
obj=malloc(sizeof(struct server));
if(obj==NULL)return NULL;
ee_t*ee=ee_new();
if(ee==NULL)return NULL;
obj->ee=ee;
obj->ch=channel_new();
if(obj->ch==NULL){
printf("obj->ch is NULL\n");	
ee_destroy(ee);
obj->ee=NULL;
return NULL;
}
obj->ch->ee=obj->ee;
obj->emit=emiti;
obj->name=NULL;
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
s->state=0;	
s->conn=serv;
//s->name=NULL;
//s->result=NULL;
}
void soup_bind_callback(struct soup*soupi, void (*cb)(struct soup*,void*),void*arg){
//if(s->cb !=NULL) //fatal error : already bound
soupi->cb=cb;
soupi->arg=arg;
}
int make_room(struct soup*soupi, char*method){
	printf("make room occured.\n");
if(!soupi)return 0;
if(!method)return 0;
if(soupi->conn==NULL) return 0;	
soupi->name=strdup("worker.createRoom");

soupi->conn->create_room(soupi->conn, 2, soupi);
return 1;
}

