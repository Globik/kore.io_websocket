#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include "soup_server.h"

const char*zweite_data="zweite_data";
static void closi(struct server*);


void on_zweite_data(void*data){
printf("on_zweite_data: %s\n",(char*)data);
//here create a room instance
on_funi("me too");//??
}

void ersti_cb(struct channel*ch, void*str){
printf("ersti cb occured.\n");
struct responsi *resp=(struct responsi*)str;
printf("here data: %s\n",(char*)resp->data);
//ch->on_ersti=NULL;
on_funi(resp->data);
}

int create_room(struct server * server, int a, on_funny on_funi){
printf("in create_room()\n");
//ee_once(server->ch->ee, zweite_data, on_zweite_data);//on accepted with data if any, rejected
server->ch->request(server->ch,"create_room", ersti_cb);
return 0;
}

static void emiti(struct server*server,const char*str,void*data){
ee_emit(server->ee, str, data);	
}

struct server*server_new(ee_t*ee,struct channel*ch){
if(ch==NULL)return NULL;
struct server*obj=NULL;
obj=malloc(sizeof(struct server));
if(obj==NULL)return NULL;
obj->ee=ee;
obj->ch=ch;
ch->ee=obj->ee;
obj->emit=emiti;
obj->close=closi;
obj->create_something=create_somethingi;
return obj;
}

void closi(struct server*obj){
printf("closi occured\n");
struct out_data data;
data.str="someone closes the server.";
obj->emit(obj,"close",(void*)&data);
}
