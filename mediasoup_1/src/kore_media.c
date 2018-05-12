#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
//#include "ref_counter.h"
#include <jansson.h>
#include "uv_callback.h"
#include "kore_media.h"
#include "soup_utils.h" //uint32_t random_u32();
#include "Channel/UnixStreamSocket.hpp"
#include <kore/kore.h>
/*
const char*room_mediaCodecs :
		[
			{
				kind        : 'audio',
				name        : 'audio/opus',
				clockRate   : 48000,
				payloadType : 100,
				numChannels : 2
			},
			{
				kind        : 'audio',
				name        : 'audio/PCMU',
				payloadType : 0,
				clockRate   : 8000
			},
			{
				kind        : 'video',
				name        : 'video/vp8',
				payloadType : 110,
				clockRate   : 90000
			},
			{
				kind       : 'video',
				name       : 'video/h264',
				clockRate  : 90000,
				payloadType : 112,
				parameters :
				{
					packetizationMode : 1
				}
			},
			{
				kind        : 'depth',
				name        : 'video/vp8',
				payloadType : 120,
				clockRate   : 90000
			}
		]
*/

static gint vid = 0;
uv_callback_t to_cpp;
int refdebug=1;
static void on_room_remove(gpointer);
static void add_room_to_media(struct media*, struct room*);
static int create_room(struct media*,char*);
//static void notify(struct media*,char*);
static void destroy_media(const j_refcnt*refp);
static void destroy_room(const j_refcnt*refp);
static struct room*get_room(struct media*,gint);
static void close_room(struct media*, gint);


static void on_room_remove(gpointer data)
{
printf("on_room_remove occured.\n");
struct room * r = (struct room*)data;
if(!r)return;
printf("data:%p %d\n",r,r->a);
j_refcount_dec(&r->ref);
}

void add_room_to_media(struct media * m, struct room * r)
{
if(m==NULL || r==NULL)return;
printf("adding room to media\n");
if(m->rooms==NULL){
m->rooms=g_hash_table_new_full(g_int_hash, g_int_equal, NULL, (GDestroyNotify)on_room_remove);
}
// int a = 0 int key=g_new0(gint,1) *key=tmp; insert(key);
g_hash_table_insert(m->rooms,&(r->b), r);
m->on_new_room(m,"new room!!!!!\n");
}

static int create_room(struct media*m,char*s)
{
if(m==NULL){printf("is null\n");return -1;}
printf("creating a room. : %s\n",s);
//notify(m,"room_created - success");
//ee_emit(ee, event_hello,"room_create");
uint32_t req_id=random_u32();
uint32_t room_id=random_u32();

json_auto_t * repli=json_object();
json_object_set_new(repli,"id",json_integer(req_id));
json_object_set_new(repli, "method", json_string("worker.createRoom"));
	
json_t*repli_internal=json_object();
json_object_set_new(repli_internal,"roomId",json_integer(room_id));
json_object_set_new(repli_internal,"sister",json_string("sister_1"));
	
json_auto_t*repli_data=json_object();
json_object_set_new(repli_data,"a",json_integer(1));
	
json_object_set_new(repli,"internal", repli_internal);
json_object_set_new(repli,"data",repli_data);
	/*
size_t size=json_dumpb(repli,NULL,0,0);
	if(size==0)return 0;
	char*buf=alloca(size);
	
	size=json_dumpb(repli,buf,size,0);
	*/
	char*mbuf=NULL;
	mbuf=json_dumps(repli,12);
	
	//struct json_buffer *jsi=NULL;
	//jsi=malloc(sizeof(struct json_buffer*));//kore_malloc(sizeof(*jsi));
	//jsi->json_buf=alloca(size);
	//memset(jsi->json_buf,0,size);
	//size=json_dumpb(repli,jsi->json_buf,size,0);
	/*jsi->json_len=0;
	jsi->json_buf=NULL;
	jsi->json_len=size;
	//char *dop;
	//dop=buf;
	//memset(jsi->json_buf,0,size);
	//jsi->json_buf=buf;
	jsi->json_buf=malloc(sizeof(jsi->json_buf)*size);
	if(jsi->json_buf==NULL)return -1;
	memcpy(jsi->json_buf,buf,size);
	
	//char*wl=strdup(tmp.c_str());
	//jsi->json_buf=strdup(buf);
	//memset(jsi.json_buf,0, size);
	//using memcpy to copy sructure memcpy(&person_copy,&person,sizeof(person))
	//memcpy(jsi.json_buf,buf,size);
	printf("json_size :%d \n",jsi->json_len);
	printf("json buf: %s\n",jsi->json_buf);
	*/
int rc=uv_callback_fire(&to_cpp,mbuf, NULL);
//	int rc=uv_callback_fire(&to_cpp,jsi, NULL);
printf("TO_CPP buffer: %s\n",mbuf);
printf("uv_callback_t &to_cpp fire %d",rc);
	//free(jsi->json_buf);
	//free(jsi->json_buf);
	//kore_free(jsi->json_buf);
	//free(mbuf);
	//mbuf=NULL;
	//json_decref(repli);
return 0;
}

static void destroy_media(const j_refcnt*refp)
{
g_print("Trying destroy  media.\n");
struct media*m=j_refcount(refp,struct media, ref);

g_print("boo-------------------------------------------------------- %p\n",m);
if(m->rooms !=NULL){
//j_refcount_dec(&(m->rooms)->ref);
g_hash_table_destroy(m->rooms);
}
m->rooms=NULL;
free(m);
m=NULL;
g_print(" p in media %p\n", m);
}

static void destroy_room(const j_refcnt*refp)
{
g_print("Trying destroy room.============================================================================\n");
struct room*r=j_refcount(refp,struct room, ref);
	if(!r && r==NULL)return;
	//if(ee)	ee_emit(ee,event_hello,"room_closed");

free(r);
r=NULL;
}

static struct room*get_room(struct media*m, gint vint)
{
if(!vint)return NULL;
struct room*r=NULL;
if(m!=NULL){
r = g_hash_table_lookup(m->rooms, &(vint));
if(r){
printf("ROOM FOUND! %p\n", r);
printf("result->a: %d\n", r->b);
return r;
}else{
printf("ROOM NOT FOUND!\n");
}
}
return r;
}

static void close_room(struct media*m, gint vint)
{
if(m==NULL)return;
if(m->rooms==NULL)return;
gboolean foo = g_hash_table_remove(m->rooms, &(vint));
printf("the value 'dummy_key' was %s found and removed\n", foo ? "":"not");
printf("there are %d keys in the hash.\n", g_hash_table_size(m->rooms));
//if(ee)	ee_emit(ee,event_hello,"room_closed");
}

struct media * media_new(proto_notify cb, proto_on_new_room on_new_room_cb){
struct media * m=(struct media*)calloc(1,sizeof*m);
if(m==NULL){printf("m is null\n");return NULL;}
printf("MEDIA p %p\n",m);
m->a=1;
m->rooms=NULL;
m->notify=cb;
m->create_room=create_room;

m->get_room=get_room;
m->close_room=close_room;
m->on_new_room=on_new_room_cb;

j_refcount_init(&m->ref, destroy_media);
return m;
}

struct room*room_new(){
struct room*r=(struct room*)calloc(1,sizeof*r);
if(r==NULL)return NULL;
vid++;
r->b=vid;
r->a=10;
j_refcount_init(&r->ref, destroy_room);
return r;
}
