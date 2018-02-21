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



#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
#include <limits.h>
//include <glib.h>
#include <pthread.h>
#include <jansson.h>
#include "assets.h"
#include "plugin.h"
#include "helper.h"
//#define WEBSOCKET_PAYLOAD_SINGLE	125
//#define WEBSOCKET_PAYLOAD_EXTEND_1	126
//#define WEBSOCKET_PAYLOAD_EXTEND_2	127


#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"

#define SHLIB_EXT ".so"

static GMainContext *sess_watchdog_ctx=NULL;
j_plugin * janus_plugin=NULL;
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
typedef struct{
int id;
int b;
}ex;
typedef struct{
	int a;
	void*data;
}suka;
void websocket_connect(struct connection *c)
{
	suka*m=malloc(sizeof(suka));
	m->a=1;
	m->data=NULL;
	
	ex*l=malloc(sizeof(ex));
	l->id=3;
	l->b=4;
	
	//m->data=malloc(sizeof(m->data));
	m->data=l;
	l=m->data;
	int val=l->b;
	int val2=l->id;
	
	//int val3=m->data->l->b;
	printf("GGGGGGG %d %d\n",val,val2);
	
	c->hdlr_extra=l;
	
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

//guint64 session_id = 0, handle_id = 0;
//session_id=janus_random_uint64();
	
//if(session_id == 0 && handle_id == 0){
//janus_session *session = janus_session_create(session_id);
//if(session == NULL) {kore_log(LOG_NOTICE,"session is NULL");}
	
//}
	
	
	/*
	json_t *reply=json_object();
	json_object_set_new(reply,"type",json_string("message"));
	
	json_object_set_new(reply,"msg",json_string("Hallo jason!"));
	size_t size=json_dumpb(reply,NULL,0,0);
	if(size==0){printf("Size is null\n");}
	char*buf=alloca(size);
	size=json_dumpb(reply,buf,size,0);
	printf("buffer: %s\n",buf);
	kore_websocket_send(c, 1, buf,size);
	*/
	//json_decref(reply);
	//free((charbuf);
	
}
void websocket_message(struct connection *c, u_int8_t op, void *data, size_t len)
{
	if(data==NULL) return;
	//kore_log(LOG_NOTICE,"some message: %s",(char*)data);
	ex * r=c->hdlr_extra;
	printf("ON message: %d id: %d\n",r->b,r->id);
	
	kore_websocket_broadcast_room(c,op,data,len,1);
	fwrite((char*)data,1,len,stdout);
	printf("\n");

			//	kore_websocket_send(c, 1, offer,strlen(offer));
			//kore_websocket_send(c, 1, offer_sdp,strlen(offer_sdp));
	
	
	/*
	
	json_t *root = load_json((const char*)data, len);
	if(root){
	 char*foo=json_dumps(root,0);
	kore_log(LOG_NOTICE,"incoming message: %s",foo);
		int send_to_clients=0;
	
	json_t *t=json_object_get(root,"type");
	const char*t_txt=json_string_value(t);
		if(!strcmp(t_txt,"message")){
		kore_log(LOG_NOTICE,"type message");
	
		}else if(!strcmp(t_txt,"login")){
		kore_log(LOG_NOTICE,"type login");
		}else if(!strcmp(t_txt,"candidate")){
		kore_log(LOG_NOTICE,"type candidate.");
		//handle_candidate(root);
		json_t*f=json_object_get(root,"cand");
		char*fu=json_string_value(f);
		printf("HERE CANDIDATE: %s\n",fu);
			char *fuc="a=candidate:1 1 UDP 2013266431 10.34.66.177 53484 typ host\r\n";
			handle_candidate(fu);
			//lllsdss
		send_to_clients=1;
		}else if(!strcmp(t_txt,"offer")){
		kore_log(LOG_NOTICE,"type offer");
        handle_offer(root,c);	
		send_to_clients=1;
		}else if(!strcmp(t_txt,"answer")){
		kore_log(LOG_NOTICE,"type answer");
		
			
		handle_answer(root);
		send_to_clients=1;
		}else{
		kore_log(LOG_NOTICE,"unknown type");
		send_to_clients=1;
		}
		
if(send_to_clients==0)kore_websocket_send(c,op,data,len);	
free(foo);
	}else{}

	json_decref(root);
	*/
}

void
websocket_disconnect(struct connection *c)
{
	c->hdlr_extra=NULL;
	kore_log(LOG_NOTICE, "%p: disconnecting", c);
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
/*

static gboolean j_check_sess(gpointer user_data){
	g_print("tick-tack\n");
return G_SOURCE_CONTINUE;
}
*/
//static 
volatile gint stop=0;
//static 
gint stop_signal=0;
/*
static gint is_stopping(void){return g_atomic_int_get(&stop);}
ddd
static void j_handle_signal(int signum) {
	stop_signal = signum;
	switch(g_atomic_int_get(&stop)) {
		case 0:jjjj
			g_print("Stopping gateway, please wait...\n");
			break;
		case 1:
			g_print("In a hurry? I'm trying to free resources cleanly, here!\n");
			break;
		default:
			g_print("Ok, leaving immediately...\n");
			break;
	}
	g_atomic_int_inc(&stop);
	if(g_atomic_int_get(&stop) > 2) {
		g_print("here must be exit(1)\n");
		exit(1);
	}
		
}
*/
/*

static gpointer j_sess_watchdog(gpointer user_data){
GMainLoop *loop=(GMainLoop *)user_data;
	GMainContext *watchdog_ctx=g_main_loop_get_context(loop);
	GSource *timeout_source=g_timeout_source_new_seconds(2);
	g_source_set_callback(timeout_source,j_check_sess,watchdog_ctx,NULL);
	g_source_attach(timeout_source,watchdog_ctx);
	g_source_unref(timeout_source);
	printf("sess watchdog started\n");
	g_main_loop_run(loop);
	return NULL;
}
*/
static void j_termination_handler(void) {g_print("at exit handler occured.\n");}


int rtc_loop(struct kore_task*t){
	signal(SIGINT, j_handle_signal);
	signal(SIGTERM, j_handle_signal);
	atexit(j_termination_handler);

	g_print(" Setup Glib \n");
	
	sess_watchdog_ctx=g_main_context_new();
	GMainLoop *watchdog_loop=g_main_loop_new(sess_watchdog_ctx,FALSE);
	GError *err=NULL;
	GThread *watchdog=g_thread_try_new("sess",&j_sess_watchdog,watchdog_loop,&err);
	if(err !=NULL){
	printf("fatal err trying to start sess watchdog\n");
	//exit(1);
		return (KORE_RESULT_OK);
	}
	
	
	
	struct dirent *pluginent = NULL;
	const char *path=NULL;
	DIR *dir=NULL;
	path="/home/globik/kore.io_websocket/websocket_8/plugin";
	
	g_print("Plugins folder: %s\n", path);
	dir = opendir(path);
	if(!dir) {
		g_print("\tCouldn't access plugins folder...\n");
		exit(1);
	}
	
	char pluginpath[1024];
	while((pluginent = readdir(dir))) {
		int len = strlen(pluginent->d_name);
		if (len < 4) {
			continue;
		}
		if (strcasecmp(pluginent->d_name+len-strlen(SHLIB_EXT), SHLIB_EXT)) {
			continue;
		}
		
		g_print("LOADING PLUGIN %s\n",pluginent->d_name);
		
		memset(pluginpath, 0, 1024);
		g_snprintf(pluginpath, 1024, "%s/%s", path, pluginent->d_name);
		void *plugin = dlopen(pluginpath, RTLD_NOW | RTLD_GLOBAL);
		if (!plugin) {
			g_print("\tCouldn't load plugin '%s': %s\n", pluginent->d_name, dlerror());
		} else {
			create_p *create = (create_p*) dlsym(plugin, "plugin_create");
			const char *dlsym_error = dlerror();
			if (dlsym_error) {
				g_print( "\tCouldn't load symbol 'plugin_create': %s\n", dlsym_error);
				continue;
			}
			
	
				janus_plugin = create();
			if(!janus_plugin) {
				g_print("\tCouldn't use function 'plugin_create'...\n");
				continue;
			}
			/* Are all the mandatory methods and callbacks implemented? */
			if(!janus_plugin->init || !janus_plugin->destroy ||!janus_plugin->handle_message ) {
				g_print("\tMissing some mandatory methods/callbacks, skipping this plugin...\n");
				continue;
			}
		
			if(janus_plugin->init(&j_handler_plugin) < 0) {
				g_print( "The  plugin could not be initialized\n");
				dlclose(plugin);
				continue;
			}
			
		}
	}
	
	
	closedir(dir);
	
	
	
	
	
	while(!g_atomic_int_get(&stop)){
	usleep(250000);
	}
	
	
	g_print("ending watchdog loop\n");
	g_main_loop_quit(watchdog_loop);
	g_thread_join(watchdog);
	watchdog=NULL;
	g_main_loop_unref(watchdog_loop);
	g_main_context_unref(sess_watchdog_ctx);
	sess_watchdog_ctx=NULL;
	if(janus_plugin !=NULL) janus_plugin->destroy();
	g_print("Bye!\n");
	//exit(0);
	//kore_task_channel_write(t,"mama\0",5);
return (KORE_RESULT_OK);
}