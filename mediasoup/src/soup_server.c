#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <kore/kore.h>
#include "soup_server.h"


static void closi(struct server*);

int create_room(struct server*, int, struct soup*);


void md_destroy(struct server*);

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
	if(serv==NULL){printf("serv is NULL.\n");return;}
	printf("md_destroy occured for mediasoup client.\n");
	
	if(serv->ch){
	printf("looks like serv->ch still there\n");
	serv->ch->close(serv->ch);//free channel
	serv->ch=NULL;
	}
	if(serv->name){printf("looks like serv->name still there.\n");free(serv->name);serv->name=NULL;}
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

const char*room_options_2="{\"mediaCodecs\":[{\"kind\":\"audio\",\"name\":\"audio/opus\",\"clockRate\":48000,\"payloadType\":100,\"numChannels\":2},{\"kind\":\"audio\",\"name\":\"audio/PCMU\",\"payloadType\":0,\"clockRate\":8000},{\"kind\":\"video\",\"name\":\"video/vp8\",\"payloadType\":110,\"clockRate\":90000},{\"kind\":\"video\",\"name\":\"video/h264\",\"clockRate\":90000,\"payloadType\":112,\"parameters\":{\"packetizationMode\":1}},{\"kind\":\"depth\",\"name\":\"video/vp8\",\"payloadType\":120,\"clockRate\": 90000}]}";


void soup_init(struct soup*s,struct server*serv){
	//if serv == NULL return;
memset(s, 0, sizeof(*s));
s->state= SOUP_STATE_INIT;	
s->conn=serv;
//s->name=NULL;//?? by memset scenario causes the segfault
//s->result=NULL;//?? by memset scenario is segfault
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
soupi->name=kore_strdup("worker.createRoom");

soupi->conn->create_room(soupi->conn, 2, soupi);
return 1;
}

