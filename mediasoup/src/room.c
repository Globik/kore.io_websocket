#include <inttypes.h>
#include <kore/kore.h>
//#include <ee.h>
#include "soup_server.h"
#include "channel.h"
#include "room.h"


#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"

static void on_room_targetId(void*);
//void set_event();
void on_room_targetId(void* rdata){
printf("on_room_targetId()\n");	
//printf("data event:\n");
//printf("data: %s\n",(char*)rdata);
//by idea in for loop free room where target id == roomid
printf("HOLDER.A: %d\n", holder.a);
}
struct room* room_new(uint32_t room_id, struct channel* ch)//internal, data, channel
{
	if(!ch){kore_log(LOG_INFO, "no channel?");return NULL;}
	struct room* r=malloc(sizeof(struct room));
	if(r==NULL)return NULL;
	r->name="globik";
	r->roomId = room_id;
	r->ch=ch;
	r->befree=room_free;
char stri_uint[9];
snprintf(stri_uint, sizeof stri_uint,"%" PRIu32, room_id);
kore_log(LOG_INFO, red "stri: %s" rst, stri_uint);
char*foo=stri_uint;
ee_on(ch->ee, foo, on_room_targetId);
//r->event = set_event;
return r;
}


void room_free(struct room* r){
r->roomId=0;
r->ch=NULL;
free(r);
r=NULL;	
}
/*
void set_event(struct channel* ch){
ee_on(ch->ee, ev_str, on_room_targetId);
}*/
