/*
 * Copyright (c) 2014 Joris Vink <joris@coders.se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BjjE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OmjiR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <unistd.h> // usleep
#include <dlfcn.h>
#include <dirent.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <signal.h>



//#include <dlfcn.h>
#include <dirent.h>
//#include <net/if.h>
#include <netdb.h>
//#include <signal.h>
//#include <getopt.h>
#include <sys/resource.h>
//#include <sys/stat.h>
//#include <poll.h>



#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
#include <limits.h>
//include <glib.h>
#include <pthread.h>
#include <jansson.h>
#include "assets.h"


#include "janus.h"
//#include "version.h"ssssssssssddsskkss
//#include "cmdline.h"
//#include "config.h"
#include "apierror.h"
//#include "log.h"
//#include "debug.h"
//#include "ip-utils.h"
#include "rtcp.h"
#include "auth.h"
#include "record.h"
#include "helper.h"

//#define WEBSOCKET_PAYLOAD_SINGLE	125
//#define WEBSOCKET_PAYLOAD_EXTEND_1	126
//#define WEBSOCKET_PAYLOAD_EXTEND_2	127hhhddssmdqsss
//kxss

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"

#define SHLIB_EXT ".so"

janus_config *config = NULL;
//static dd
	//char *config_file = NULL;kssde
//char *configs_folder = NULL;
	GHashTable *transports = NULL;
	GHashTable *transports_so = NULL;
	GHashTable *eventhandlers = NULL;
	GHashTable *eventhandlers_so = NULL;
GHashTable *plugins = NULL;
//static 
	GHashTable *plugins_so = NULL;
gboolean daemonize = FALSE;
int pipefd[2];

char *api_secret = NULL, *admin_api_secret = NULL;
gchar *local_ip = NULL;
//static 
gchar *public_ip = NULL;
volatile gint stop = 0;gint stop_signal = 0;
gchar *server_name = NULL;
//static 
uint session_timeout = DEFAULT_SESSION_TIMEOUT;
int janus_log_level = LOG_INFO;
gboolean janus_log_timestamps = FALSE;
gboolean janus_log_colors = FALSE;
int lock_debug = 0;
//static 
	gint initialized = 0, stopping = 0;
//static 
	janus_transport_callbacks *gateway = NULL;
//static
	gboolean ws_janus_api_enabled = FALSE;
//static 
	gboolean ws_admin_api_enabled = FALSE;
//static 
	gboolean notify_events = TRUE;
//static 
	size_t json_format = JSON_INDENT(3) | JSON_PRESERVE_ORDER;
//static 
	janus_mutex sessions_mutex;
//static 
	GHashTable *sessions = NULL, *old_sessions = NULL;
//static 
	GMainContext *sessions_watchdog_context = NULL;


struct kore_task pipe_task;

int do_loop(struct kore_task*);
int run_curl(struct kore_task*);
int		page(struct http_request *);
int		page_ws_connect(struct http_request *);

void		websocket_connect(struct connection *);
void		websocket_disconnect(struct connection *);
void		websocket_message(struct connection *, u_int8_t, void *, size_t);
json_t *load_json(const char *, size_t);


int init(int);
int pipe_reader(struct kore_task *);
int rtc_loop(struct kore_task *);
void pipe_data_available(struct kore_task *);
int inc_req(json_t);


int init(state){
	printf("Entering init.\n");
if(state==KORE_MODULE_UNLOAD) return (KORE_RESULT_ERROR);
	//if(worker->id !=1) return (KORE_RESULT_OK);
	printf("after state.\n");
	//kore_task_create(&pipe_task,pipe_reader);
	kore_task_create(&pipe_task,rtc_loop);
	kore_task_bind_callback(&pipe_task, pipe_data_available);
	kore_task_run(&pipe_task);
	return (KORE_RESULT_OK);
}

json_t *load_json(const char *text,size_t buflen) {
    json_t *root;
    json_error_t error;

    root = json_loadb(text, buflen, 0, &error);

    if (root) {
        return root;
    } else {
        fprintf(stderr, "json error on line %d: %s\n", error.line, error.text);
        return (json_t *)0;
    }
}


int ab=0;
 struct ex{
int id;
int b;
};
typedef struct{
	int a;
	void*data;
}suka;
void websocket_connect(struct connection *c)
{
	ab++;
	struct ex *l;
l=kore_malloc(sizeof(*l));
	l->id=ab;
	l->b=46;
	
	//m->data=malloc(sizeof(m->data));
	//m->data=l;
	//l=m->data;
	//int val=l->b;
	//int val2=l->id;
	
	//int val3=m->data->l->b;
	//printf("GGGGGGG %d %d\n",val,val2);
	
	c->hdlr_extra=l;
	//free(l);
	//free(m);
	
		//m->data=kore_malloc(sizeof(m->data));
	//m->data=l;
	//m->data->l->b++;
	//printf("%d\n",m->data->l->b);
	
	
	/*
	ab++;
	char*room="alice";
	//struct extra *lik=kore_malloc(sizeof(*lik));
	struct extra lik={1,"mama"};
	//lik->uid=ab;
	//lik->room=room;
c->hdlr_extra=&lik;
	c->hdlr_extra&(lik.uid++);
	*/

//printf("rom: %s\n",c->hdlr_extra->lik.room);
//kore_log(LOG_NOTICE, "%p: connected, by name %s: ", c,(char*)c->hdlr_extra->room);

guint64 session_id = 0, handle_id = 0;
if(session_id==0 && handle_id==0){
janus_session *session=janus_session_create(session_id);
if(session==NULL){kore_log(LOG_NOTICE,"session is NULL!");}
session_id=session->session_id;
}
	
if(session_id > 0 && janus_session_find(session_id) !=NULL){
kore_log(LOG_NOTICE,"The session already in use");
}
	

	
	
	
	json_auto_t *reply=json_object();
	json_object_set_new(reply,"type",json_string("user_id"));
	json_object_set_new(reply,"msg",json_string("Hallo jason!"));
	json_object_set_new(reply,"id",json_integer(l->id));
	json_object_set_new(reply,"b",json_integer(l->b));
	json_object_set_new(reply,"vid",json_integer(session_id));
	json_object_set_new(reply,"transaction",json_string("fuck_transaction"));
	size_t size=json_dumpb(reply,NULL,0,0);
	if(size==0){printf("Size is null\n");}
	//char*buf=alloca(size);
	char*buf=kore_malloc(size);
	size=json_dumpb(reply,buf,size,0);
	printf("buffer: %s\n",buf);
	kore_websocket_send(c, 1, buf,size);
	
	//json_decref(reply);
	//kore_buf_free(buf);
	//kore_free(l);
	kore_free(buf);
	
}
void websocket_message(struct connection *c, u_int8_t op, void *data, size_t len)
{
	if(data==NULL) return;
	//kore_log(LOG_NOTICE,"some message: %s",(char*)data);
	struct ex * r=c->hdlr_extra;
	printf("ON message: %d id: %d\n",r->b,r->id);
	
	//kore_websocket_broadcast_room(c,op,data,len,1);
	fwrite((char*)data,1,len,stdout);
	printf("\n");

			//	kore_websocket_send(c, 1, offer,strlen(offer));
			//kore_websocket_send(c, 1, offer_sdp,strlen(offer_sdp));
	
	
	
	
	json_auto_t *root = load_json((const char*)data, len);
	if(root){
	 char*foo=json_dumps(root,0);
	kore_log(LOG_NOTICE,"incoming message: %s",foo);
		int send_to_clients=0;
		
		guint64 session_id=0,handle_id=0;
		json_t *s=json_object_get(root,"vid");
		if(s && json_is_integer(s)) session_id=json_integer_value(s);
		json_t *h=json_object_get(root,"handle_id");
		if(h && json_is_integer(h)) handle_id=json_integer_value(h);
		
		janus_session *session=janus_session_find(session_id);
		if(!session){
		kore_log(LOG_NOTICE,"The session not found. Returning");return;
		}
		session->last_activity=janus_get_monotonic_time();
		janus_ice_handle *handle=NULL;
	
	json_t *t=json_object_get(root,"type");
	const char*t_txt=json_string_value(t);
		if(!strcmp(t_txt,"message")){
		kore_log(LOG_NOTICE,"type message");
	
		}else if(!strcmp(t_txt,"login")){
		kore_log(LOG_NOTICE,"type login");
		}else if(!strcmp(t_txt,"attach")){
		kore_log(LOG_NOTICE,"type attach.");
		//json_t*f=json_object_get(root,"cand");
		//char*fu=json_string_value(f);
		//printf(": %s\n",fu);
		send_to_clients=1;
		}else if(!strcmp(t_txt,"detach")){
		kore_log(LOG_NOTICE,"type detach");
       
		send_to_clients=1;
		}else if(!strcmp(t_txt,"answer")){
		kore_log(LOG_NOTICE,"type answer");
		send_to_clients=1;
		}else{
		kore_log(LOG_NOTICE,"unknown type");
		send_to_clients=1;
		}
		
//if(send_to_clients==0)
	kore_websocket_send(c,op,data,len);	
free(foo);
	}else{}

	//json_decref(root);
	
}

void
websocket_disconnect(struct connection *c)
{
	//c->hdlr_extra=NULL;
	kore_log(LOG_NOTICE, "%p: disconnecting", c);
	struct ex*l=c->hdlr_extra;
	printf("on disconnect: b: %d id: %d\n",l->id,l->b);
	ab--;
	kore_free(l);
c->hdlr_extra=NULL;
}

int page(struct http_request *req)
{
	http_response_header(req, "content-type", "text/html");
	http_response(req, 200, asset_frontend_html, asset_len_frontend_html);
	return (KORE_RESULT_OK);
}


int page_ws_connect(struct http_request *req)
{
	//req->hdlr_extra=0;
	/* Perform the websocket handshake, passing our callbacks. */
	kore_log(LOG_NOTICE,"some path %s",req->path);
	kore_log(LOG_NOTICE, "%p: http_request", req);
	kore_websocket_handshake(req, "websocket_connect","websocket_message","websocket_disconnect");

	return (KORE_RESULT_OK);
}

void pipe_data_available(struct kore_task *t){
	size_t len;
	u_int8_t buf[BUFSIZ];
	
if(kore_task_finished(t)){
kore_log(LOG_NOTICE,"Task finished.");
//return;
}

	len=kore_task_channel_read(t,buf,sizeof(buf));
	if(len > sizeof(buf)){printf("len great than buf\n");}
	kore_log(LOG_NOTICE,"TTTTTTTTTTTTTTTTTTTTTTTTTTTTTask msg: %s",buf);
	
}

/* GLIB */

/*
int j_plugin_push_event(j_plugin*plugin,const char*transaction);

static j_cbs j_handler_plugin={
.push_event=j_plugin_push_event,
};

int j_plugin_push_event(j_plugin *plugin,const char*transaction){
	if(!plugin) return -1;
	const char*mu="alice";
	const char*mud="bob";
g_print("TRANSACTION: %s\n",transaction); 
	//kore_websocket_broadcast(NULL,WEBSOCKET_OP_TEXT,"papa\0",5,WEBSOCKET_BROADCAST_GLOBAL);
	kore_websocket_broadcast_room_char(mu, WEBSOCKET_OP_TEXT, "papa\0",5, WEBSOCKET_BROADCAST_GLOBAL);
	kore_websocket_broadcast_room_char(mu, WEBSOCKET_OP_TEXT, transaction,strlen(transaction), WEBSOCKET_BROADCAST_GLOBAL);
return 0;
}
*/
int rtc_loop(struct kore_task*t){
	fuck_up();
	//kore_task_channel_write(t,"mama\0",5);
return (KORE_RESULT_OK);
}