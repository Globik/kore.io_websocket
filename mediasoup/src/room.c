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
static void room_close(struct room*);
static void on_room_targetId(void*, void*const ObjThis);
//void set_event();
//const
 struct passData{struct channel*ch;struct room* r;char* str;};
void on_room_targetId(void* rdata, void* const ObjThis){
if(ObjThis==NULL){printf(red "ObjThis is NULL? %s\n" rst, __FILE__); return;}
printf("on_room_targetId()\n");	
printf("data: %s\n", (char*)rdata);
//struct room* r=(struct room*)ObjThis;
//r->close(r);
struct passData *data=(struct passData*)ObjThis;
//struct room* r=data->r;
//struct channel *ch=data->ch;
ee_remove_all_listeners(data->ch->ee, data->str);
kore_free(data->str);
printf("before close data r\n");
data->r->close(data->r);
free(data);
data=NULL;
}
struct room* room_new(uint32_t room_id, struct channel* ch)//internal, data, channel
{
	if(!ch){kore_log(LOG_INFO, "no channel?");return NULL;}
	struct room* r=malloc(sizeof(struct room));
	if(r==NULL)return NULL;
	r->name="globik";
	r->roomId = room_id;
	r->ch=ch;
	r->close=room_close;
char stri_uint[9];
snprintf(stri_uint, sizeof stri_uint,"%" PRIu32, room_id);
kore_log(LOG_INFO, red "stri: %s" rst, stri_uint);
char*foo=stri_uint;
struct passData *pass = malloc(sizeof(struct passData));
pass->ch=ch;
pass->r=r;
pass->str=kore_strdup(stri_uint);
ee_on(ch->ee, foo, on_room_targetId,(void*)pass);
return r;
}


void room_close(struct room* r){
printf(green "on room_close()\n" rst);
//ee_remove_all_listeners(c
r->roomId=0;
r->ch=NULL;
free(r);
r=NULL;	
}
