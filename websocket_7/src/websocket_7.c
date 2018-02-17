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

static GMainContext *sess_watchdog_ctx=NULL;
j_plugin * plur=NULL;
j_plugin *plugin_create(void);
int plugin_init(j_cbs*cb);
void plugin_destroy(void);
struct j_plugin_res *plugin_handle_message(char*transaction);

j_plugin_res*j_plugin_res_new(j_plugin_res_type type,const char*text){
j_plugin_res *result=g_malloc(sizeof(j_plugin_res));
	result->type=type;
	result->text=text;
	return result;
}
void j_plugin_res_destroy(j_plugin_res *result){
result->text=NULL;
	g_free(result);
	g_print("j_plugin_res_destroy\n");
}

static j_plugin p_m=J_PLUGIN_INIT(
		.init=plugin_init,
		.destroy=plugin_destroy,
		.handle_message=plugin_handle_message,
		);
j_plugin *plugin_create(void){
printf("Created!\n");
return &p_m;
}
static volatile gint initialized=0,stopping=0;
static j_cbs *gw=NULL;
static GThread *handler_thread;
static void *plugin_handler(void*data);
typedef struct j_message{
char*transaction;
} j_message;
static GAsyncQueue *messages=NULL;
static j_message exit_message;



struct kore_task pipe_task;

int do_loop(struct kore_task*);
int run_curl(struct kore_task*);
int		page(struct http_request *);
int		page_ws_connect(struct http_request *);

void		websocket_connect(struct connection *);
void		websocket_disconnect(struct connection *);
void		websocket_message(struct connection *, u_int8_t, void *, size_t);
json_t *load_json(const char *, size_t);

//void kore_websocket_broadcast_room(struct connection *, u_int8_t, const void *,size_t, int);
//static void websocket_frame_build(struct kore_buf *, u_int8_t, const void *,size_t);
int init(int);
int pipe_reader(struct kore_task *);
int rtc_loop(struct kore_task *);
void pipe_data_available(struct kore_task *);


int init(state){
	printf("Entering init.\n");
if(state==KORE_MODULE_UNLOAD) return (KORE_RESULT_ERROR);
	//if(worker->id !=1) return (KORE_RESULT_OK);
	printf("after state.\n");
	//kore_task_create(&pipe_task,pipe_reader);
	kore_task_create(&pipe_task,rtc_loop);
	kore_task_bind_callback(&pipe_task,pipe_data_available);
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
/*
void
kore_websocket_broadcast_room(struct connection *src, u_int8_t op, const void *data,
    size_t len, int scope)
{
	struct connection	*c;
	struct kore_buf		*frame;

	frame = kore_buf_alloc(len);
	websocket_frame_build(frame, op, data, len);

	TAILQ_FOREACH(c, &connections, list) {
		if (c->hdlr_extra==src->hdlr_extra && c->proto == CONN_PROTO_WEBSOCKET) {
			net_send_queue(c, frame->data, frame->offset);
			net_send_flush(c);
		}
	}

	if (scope == WEBSOCKET_BROADCAST_GLOBAL) {
		kore_msg_send(KORE_MSG_WORKER_ALL,
		    KORE_MSG_WEBSOCKET, frame->data, frame->offset);
	}

	kore_buf_free(frame);
}
*/
/*
static void
websocket_frame_build(struct kore_buf *frame, u_int8_t op, const void *data,
    size_t len)
{
	u_int8_t		len_1;
	u_int16_t		len16;
	u_int64_t		len64;

	if (len > WEBSOCKET_PAYLOAD_SINGLE) {
		if (len < USHRT_MAX)
			len_1 = WEBSOCKET_PAYLOAD_EXTEND_1;
		else
			len_1 = WEBSOCKET_PAYLOAD_EXTEND_2;
	} else {
		len_1 = len;
	}

	op |= (1 << 7);
	kore_buf_append(frame, &op, sizeof(op));

	len_1 &= ~(1 << 7);
	kore_buf_append(frame, &len_1, sizeof(len_1));

	if (len_1 > WEBSOCKET_PAYLOAD_SINGLE) {
		switch (len_1) {
		case WEBSOCKET_PAYLOAD_EXTEND_1:
			net_write16((u_int8_t *)&len16, len);
			kore_buf_append(frame, &len16, sizeof(len16));
			break;
		case WEBSOCKET_PAYLOAD_EXTEND_2:
			net_write64((u_int8_t *)&len64, len);
			kore_buf_append(frame, &len64, sizeof(len64));
			break;
		}
	}

	kore_buf_append(frame, data, len);
}
*/
int32_t ab=0;
void
websocket_connect(struct connection *c)
{
	char*mumu="alice";
	char*fish="fisch";
	
	if(ab>1){
	kore_log(LOG_NOTICE, "Alice");
		c->hdlr_extra=mumu;
	}else{
	kore_log(LOG_NOTICE, "fisch");
		c->hdlr_extra=fish;
	}
	ab++;
kore_log(LOG_NOTICE, "%p: connected", c);
	j_plugin_res *resu=plur->handle_message("dudka_dudka");
	if(resu==NULL){g_print("resu is null\n");}
	if(resu->type==J_PLUGIN_OK){g_print("j_plugin_ok\n");}
	if(resu->type==J_PLUGIN_OK_WAIT){
	g_print("OK_WAIT: %s\n",resu->text);
	//kore_task_channel_write(t,(void*)resu->text,7);
	}
	j_plugin_res_destroy(resu);
	
	int res=gw->push_event(plur,"Fucker_BADEN_BADEN");
	kore_log(LOG_NOTICE,"res of gw->push_event(fucker_baden_baden): %d",res);
	
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
	kore_log(LOG_NOTICE, "%p: disconnecting", c);
}





int page(struct http_request *req)
{
	
	
	/*
	struct rstate *state;
	u_int32_t len;
	if(req->hdlr_extra==NULL){
	state=kore_malloc(sizeof(*state));
		req->hdlr_extra=state;
		kore_task_create(&state->task,run_curl);
	//	kore_task_bind_request(&state->task,req);
		kore_task_bind_request(&state->task, req);
		kore_task_run(&state->task);
		kore_task_channel_write(&state->task,"mama\0",5);
		return (KORE_RESULT_RETRY);
	}else{state=req->hdlr_extra;}
	
	if(kore_task_state(&state->task) !=KORE_TASK_STATE_FINISHED){
	http_request_sleep(req);
		return (KORE_RESULT_RETRY);
	}
	
	if(kore_task_result(&state->task) !=KORE_RESULT_OK){
	kore_task_destroy(&state->task);
		http_response(req,500,"fuu",3);
		return (KORE_RESULT_OK);
	}
	char result[64];
	len=kore_task_channel_read(&state->task,result,sizeof(result));
	if(len > sizeof(result)){
	http_response(req,500,"doo",4);
	*/
	//}else{
	
	
	
	http_response_header(req, "content-type", "text/html");
	//	kore_log(LOG_NOTICE,"task content: %s",result);
	http_response(req, 200, asset_frontend_html, asset_len_frontend_html);
	//}
//kore_task_destroy(&state->task);
	return (KORE_RESULT_OK);
}

/*
int run_curl(struct kore_task *t){
char user[64];
	u_int32_t len;
	
	len=kore_task_channel_read(t,user,sizeof(user));
	if(len>sizeof(user)) return (KORE_RESULT_ERROR);
	
	kore_log(LOG_NOTICE,"from run curl msg: %s",user);
	//fwrite(user,1,len,stdout);
	//printf("\n");
	kore_task_channel_write(t,"papa\0",5);
	return (KORE_RESULT_OK);
}
*/
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
static void plugin_message_free(j_message *msg){
	g_print("ENTERING plugin_message_free\n");
if(!msg || msg==&exit_message) return;
	g_free(msg->transaction);
	msg->transaction=NULL;
	g_free(msg);
	g_print("plugin_message_free\n");
}

int plugin_init(j_cbs *cbs){
if(g_atomic_int_get(&stopping)){return -1;}
	if(cbs==NULL){return -1;}
messages=g_async_queue_new_full((GDestroyNotify)plugin_message_free);
	gw=cbs;
	g_atomic_int_set(&initialized,1);
	GError *error=NULL;
	handler_thread=g_thread_try_new("echotest", plugin_handler,NULL,&error);
	if(error !=NULL){
	g_atomic_int_set(&initialized,0);
	printf("got error handler_thread: %d\n",error->code);
	return -1;
	}
	printf("Initialized!\n");
return 0;
}

void plugin_destroy(void){
if(!g_atomic_int_get(&initialized)) return;
	g_atomic_int_set(&stopping,1);
	g_async_queue_push(messages,&exit_message);
	if(handler_thread !=NULL){
	g_thread_join(handler_thread);
		handler_thread=NULL;
	}
	g_async_queue_unref(messages);
	messages=NULL;
	g_atomic_int_set(&initialized,0);
	g_atomic_int_set(&stopping,0);
	printf("Destroyd\n");
}

struct j_plugin_res *plugin_handle_message(char*transaction){
if(g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized))
	return j_plugin_res_new(J_PLUGIN_ERROR, g_atomic_int_get(&stopping) ? "shutting down" : "plugin not initialized");
	j_message*msg=g_malloc(sizeof(j_message));
	msg->transaction=transaction;
	g_async_queue_push(messages,msg);
	return j_plugin_res_new(J_PLUGIN_OK_WAIT,"i'm taking my time");
}
static void*plugin_handler(void*data){
j_message *msg=NULL;
	while(g_atomic_int_get(&initialized) && !g_atomic_int_get(&stopping)){
	msg=g_async_queue_pop(messages);
		if(msg==&exit_message) break;
		if(msg->transaction==NULL){
		plugin_message_free(msg);
			continue;
		}
		printf("got a message: %s\n",msg->transaction);
	//	int res=gw->push_event("Fucker");
	//	printf("res of gw->push_event(fucker): %d\n",res);
		continue;
	}
	
	g_print("leaving thread from PLUGIN_HANDLER\n");
	return NULL;
}

int j_plugin_push_event(j_plugin*plugin,const char*transaction);

static j_cbs j_handler_plugin={
.push_event=j_plugin_push_event,
};
int j_plugin_push_event(j_plugin *plugin,const char*transaction){
	if(!plugin) return -1;
printf("TRANSACTION: %s\n",transaction); // here must be "fucker" ...jj
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
static void j_termination_handler(void) {}


int rtc_loop(struct kore_task*t){
	signal(SIGINT, j_handle_signal);
	signal(SIGTERM, j_handle_signal);
	atexit(j_termination_handler);

	g_print(" Setup Glib \n");
	
	//j_plugin *
		plur=plugin_create();
	if(!plur){g_print("create plugin failer\n");
				return (KORE_RESULT_OK);
				//exit(1);
			   }
	if(plur->init(&j_handler_plugin) < 0){
	g_print("no plugin init\n");
		//exit(1);
		return (KORE_RESULT_OK);
	}
	j_plugin_res *resu=plur->handle_message("dudka");
	if(resu==NULL){g_print("resu is null\n");}
	if(resu->type==J_PLUGIN_OK){g_print("j_plugin_ok\n");}
	if(resu->type==J_PLUGIN_OK_WAIT){
	g_print("J_PLUGIN_OK_WAIT: %s\n",resu->text);
	kore_task_channel_write(t,(void*)resu->text,7);
	}
	
	int res=gw->push_event(plur,"Fucker");
	printf("res of gw->push_event(fucker): %d\n",res);
	j_plugin_res_destroy(resu);
	
	sess_watchdog_ctx=g_main_context_new();
	GMainLoop *watchdog_loop=g_main_loop_new(sess_watchdog_ctx,FALSE);
	GError *err=NULL;
	GThread *watchdog=g_thread_try_new("sess",&j_sess_watchdog,watchdog_loop,&err);
	if(err !=NULL){
	printf("fatal err trying to start sess watchdog\n");
	//exit(1);
		return (KORE_RESULT_OK);
	}
	while(!g_atomic_int_get(&stop)){
	usleep(250000);
	}
	
	
	printf("ending watchdog loop\n");
	g_main_loop_quit(watchdog_loop);
	g_thread_join(watchdog);
	watchdog=NULL;
	g_main_loop_unref(watchdog_loop);
	g_main_context_unref(sess_watchdog_ctx);
	sess_watchdog_ctx=NULL;
	plur->destroy();
	//exit(0);
	//kore_task_channel_write(t,"mama\0",5);
return (KORE_RESULT_OK);
}