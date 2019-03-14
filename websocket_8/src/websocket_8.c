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
#include <pthread.h>
#include <jansson.h>
#include "assets.h"
#include "plugin.h"
#include "helper.h"


#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"

#define SHLIB_EXT ".so"
volatile gint stop=0;
gint stop_signal=0;
static GMainContext *sess_watchdog_ctx=NULL;


j_plugin * janus_plugin=NULL;
struct kore_task pipe_task;


int		page(struct http_request *);
int		page_ws_connect(struct http_request *);

void		websocket_connect(struct connection *);
void		websocket_disconnect(struct connection *);
void		websocket_message(struct connection *, u_int8_t, void *, size_t);
json_t *load_json(const char *, size_t);
void go_handle_message(void);
void kore_worker_configure(void);

int init(int);
int pipe_reader(struct kore_task *);
int rtc_loop(struct kore_task *);

void pipe_data_available(struct kore_task *);


void received_message(struct kore_msg*,const void*);
void han(void);

int init(state){
if(state==KORE_MODULE_UNLOAD) return (KORE_RESULT_ERROR);
	(void)kore_msg_register(MY_MESSAGE_ID, received_message);
	//if(worker->id !=1) return (KORE_RESULT_OK);
	kore_log(LOG_NOTICE,yellow "[%s:%d] Worker ID %d : %p" rst,__FILE__,__LINE__,worker->id,worker);
	kore_task_create(&pipe_task,rtc_loop);
	kore_task_bind_callback(&pipe_task, pipe_data_available);
	kore_task_run(&pipe_task, 0);
	return (KORE_RESULT_OK);
}

void kore_worker_configure(){
atexit(han);
}
void han(){
kore_log(LOG_INFO,"[%s:%d] at exit worker 1 occured", __FILE__, __LINE__);
}
void kore_worker_teardown(void){
kore_log(LOG_INFO, yellow "[%s:%d] TEARDOWN!" rst, __FILE__, __LINE__);
	g_atomic_int_inc(&stop);
	usleep(400000);
}

void go_handle_message(){
j_plugin_res *resu=janus_plugin->handle_message("NACH ECHO PLUGIN MESSAGE");
	if(resu==NULL){kore_log(LOG_NOTICE,"[%s:%d] resu is null\n", __FILE__, __LINE__);}
	if(resu->type==J_PLUGIN_OK){kore_log(LOG_NOTICE,"[%s:%d] j_plugin_ok\n", __FILE__, __LINE__);}
	if(resu->type==J_PLUGIN_OK_WAIT){kore_log(LOG_NOTICE,"[%s:%d] J_PLUGIN_OK_WAIT: %s\n", __FILE__, __LINE__, resu->text);}
	//int res=gw->push_event(&p_m,"Fucker"); in echo.c plugin
	j_plugin_res_destroy(resu);
}
void received_message(struct kore_msg*msg, const void*data){
kore_log(LOG_INFO,green "[%s:%d] Got message from %u (%d bytes): %.*s" rst, __FILE__, __LINE__,  
msg->src, msg->length, msg->length, (const char*)data);
	
	if(janus_plugin==NULL){
	kore_log(LOG_INFO,"[%s:%d] JANUS_PLUGIN IS NULL!!!", __FILE__, __LINE__);
	}else{
	kore_log(LOG_INFO,"[%s:%d] JANUS_PLUGIN IS NOT NULL!! SENDING SOME INFO TO ECHO PLUGIN", __FILE__, __LINE__);
	kore_log(LOG_INFO,"[%s:%d] Worker id: %d", __FILE__, __LINE__, worker->id);
	//go_handle_message();
		
	}
	if(msg->src==1){
	kore_log(LOG_INFO,"[%s:%d] sending global for websocket", __FILE__, __LINE__);
	//kore_websocket_broadcast(NULL,WEBSOCKET_OP_TEXT,"rafa\0",5,WEBSOCKET_BROADCAST_GLOBAL);
	}
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


int32_t ab=0;
void
websocket_connect(struct connection *c)
{
char*mumu="alice";
c->hdlr_extra=mumu;
ab++;
kore_log(LOG_NOTICE, "[%s:%d] %p: connected, by name %s: ", __FILE__, __LINE__, c,(char*)c->hdlr_extra);
go_handle_message();
if(worker->id==1){
if(janus_plugin !=NULL) {
//go_handle_message();
}
}else{
kore_log(LOG_NOTICE,"[%s:%d] JANUS_PLUGIN IS NULL!!!", __FILE__, __LINE__);
//kore_msg_send(1,MY_MESSAGE_ID,"hello",5);
//kore_msg_send(0, MY_MESSAGE_ID, "hello",5);
}
}
void websocket_message(struct connection *c, u_int8_t op, void *data, size_t len)
{
	if(data==NULL){printf("ws data is NULL\n");}
	kore_websocket_broadcast_room(c,op,data,len,1);
	fwrite((char*)data,1,len,stdout);
	printf("\n");
/*
json_t *root = load_json((const char*)data, len);
if(root){
char*foo=json_dumps(root,0);
kore_log(LOG_NOTICE,"incoming message: %s",foo);	
free(foo);
json_decref(root);
}else{}
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
	kore_msg_send(0, MY_MESSAGE_ID, "PAGE!",5);
	return (KORE_RESULT_OK);
}


int page_ws_connect(struct http_request *req)
{
	kore_log(LOG_NOTICE,"some path %s",req->path);
	//kore_log(LOG_NOTICE, "%p: http_request", req);
	kore_websocket_handshake(req, "websocket_connect","websocket_message","websocket_disconnect");

	return (KORE_RESULT_OK);
}

void pipe_data_available(struct kore_task *t){
if(kore_task_finished(t)){
kore_log(LOG_NOTICE,yellow "[%s:%d] Task finished." rst, __FILE__, __LINE__);

return;
}
size_t len;
u_int8_t buf[BUFSIZ];
len=kore_task_channel_read(t,buf,sizeof(buf));
if(len > sizeof(buf)){kore_log(LOG_INFO, red "[%s:%d] len great than buf" rst, __FILE__, __LINE__);}
kore_log(LOG_NOTICE,yellow "[%s:%d] Task msg: %s" rst,__FILE__, __LINE__, buf);
}


/* GLIB */


int j_plugin_push_event(j_plugin*plugin,const char*transaction,struct kore_task*t);

static j_cbs j_handler_plugin={
.push_event=j_plugin_push_event,
};
int h=0;
int j_plugin_push_event(j_plugin *plugin,const char*transaction,struct kore_task*t){
if(!plugin) return -1;
const char*mu="alice";
	
kore_log(LOG_NOTICE,"TRANSACTION: %s\n",transaction); 
kore_log(LOG_NOTICE,"worker id, memory %d : %p",worker->id,worker);
kore_websocket_broadcast_room_char(mu, WEBSOCKET_OP_TEXT, "papa\0",5, WEBSOCKET_BROADCAST_GLOBAL);
size_t duri=strlen(transaction);
kore_msg_send(0,MY_MESSAGE_ID,"bubuk", 5);
kore_msg_send(0,MY_MESSAGE_ID, transaction, duri);
return 0;
}



static void j_termination_handler(void) {g_print("at exit handler occured.\n");}


int rtc_loop(struct kore_task*t){
	//signal(SIGINT, j_handle_signal);
	//signal(SIGTERM, j_handle_signal);
	atexit(j_termination_handler);

	g_print(" Setup Glib \n");
	kore_task_channel_write(t,"mama\0",5);
	sess_watchdog_ctx=g_main_context_new();
	GMainLoop *watchdog_loop=g_main_loop_new(sess_watchdog_ctx,FALSE);
	GError *err=NULL;
	GThread *watchdog=g_thread_try_new("sess",&j_sess_watchdog,watchdog_loop,&err);
	if(err !=NULL){
	printf("fatal err trying to start sess watchdog\n");
		return (KORE_RESULT_OK);
	}
	
	
	kore_task_channel_write(t,"mama\0",5);

	
	
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
		
		g_print("[%s:%d] LOADING PLUGIN %s\n",__FILE__,__LINE__, pluginent->d_name);
		
		memset(pluginpath, 0, 1024);
		g_snprintf(pluginpath, 1024, "%s/%s", path, pluginent->d_name);
		void *plugin = dlopen(pluginpath, RTLD_NOW | RTLD_GLOBAL);
		if (!plugin) {
			g_print(red "\t [%s:%d] Couldn't load plugin '%s': %s\n" rst,__FILE__,__LINE__, pluginent->d_name, dlerror());
		} else {
			create_p *create = (create_p*) dlsym(plugin, "plugin_create");
			const char *dlsym_error = dlerror();
			if (dlsym_error) {
				g_print( red "\t[%s:%d] Couldn't load symbol 'plugin_create': %s\n" rst,__FILE__,__LINE__, dlsym_error);
				continue;
			}
			
	
				janus_plugin = create();
			if(!janus_plugin) {
				g_print(red "\t[%s:%d] Couldn't use function 'plugin_create'...\n" rst,__FILE__,__LINE__);
				continue;
			}
			/* Are all the mandatory methods and callbacks implemented? */
			if(!janus_plugin->init || !janus_plugin->destroy ||!janus_plugin->handle_message ) {
				g_print(red "\t[%s:%d] Missing some mandatory methods/callbacks, skipping this plugin...\n" rst,__FILE__,__LINE__);
				continue;
			}
		
			if(janus_plugin->init(&j_handler_plugin,t) < 0) {
				g_print(red "[%s:%d] The  plugin could not be initialized\n" rst,__FILE__,__LINE__);
				dlclose(plugin);
				continue;
			}
			
		}
	}
	closedir(dir);
	while(!g_atomic_int_get(&stop)){
	usleep(250000);
	}
	g_print(yellow "[%s:%d] ending watchdog loop\n" rst,__FILE__,__LINE__);
	g_main_loop_quit(watchdog_loop);
	g_thread_join(watchdog);
	watchdog=NULL;
	g_main_loop_unref(watchdog_loop);
	g_main_context_unref(sess_watchdog_ctx);
	sess_watchdog_ctx=NULL;
	if(janus_plugin !=NULL) {janus_plugin->destroy();janus_plugin=NULL;}
	g_print(yellow "[%s:%d] Bye!\n" rst,__FILE__,__LINE__);
return (KORE_RESULT_OK);
}
