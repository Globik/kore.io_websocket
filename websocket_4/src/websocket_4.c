/*
 * Copyright (c) 2014 Joris Vink <joris@coders.se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
#include <limits.h>
#include <glib.h>
#include <pthread.h>
#include <jansson.h>
#include "assets.h"

#include "rtcdc.h"

#define WEBSOCKET_PAYLOAD_SINGLE	125
#define WEBSOCKET_PAYLOAD_EXTEND_1	126
#define WEBSOCKET_PAYLOAD_EXTEND_2	127
//struct rstate{struct kore_task task;};
struct kore_task pipe_task;
GMainLoop*loop;
gint count=10;
gboolean cb(gpointer);
int do_loop(struct kore_task*);
int run_curl(struct kore_task*);
int		page(struct http_request *);
int		page_ws_connect(struct http_request *);

void		websocket_connect(struct connection *);
void		websocket_disconnect(struct connection *);
void		websocket_message(struct connection *, u_int8_t, void *, size_t);
json_t *load_json(const char *, size_t);

void kore_websocket_broadcast_room(struct connection *, u_int8_t, const void *,size_t, int);
static void websocket_frame_build(struct kore_buf *, u_int8_t, const void *,size_t);
int init(int);
int pipe_reader(struct kore_task *);
int rtc_loop(struct kore_task *);
void pipe_data_available(struct kore_task *);
/* Websocket callbacks. 
struct kore_wscbs wscbs = {
	websocket_connect,
	websocket_message,
	websocket_disconnect
};*/

int init(state){
	printf("SUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUKA\n");
if(state==KORE_MODULE_UNLOAD) return (KORE_RESULT_ERROR);
	//if(worker->id !=1) return (KORE_RESULT_OK);
	printf("sukkkkkkkkkkkkka2\n");
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

void
kore_websocket_broadcast_room(struct connection *src, u_int8_t op, const void *data,
    size_t len, int scope)
{
	struct connection	*c;
	struct kore_buf		*frame;

	frame = kore_buf_alloc(len);
	websocket_frame_build(frame, op, data, len);

	TAILQ_FOREACH(c, &connections, list) {
		if (/*c != src && */ c->hdlr_extra==src->hdlr_extra && c->proto == CONN_PROTO_WEBSOCKET) {
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
/* Called whenever we get a new websocket connection. */
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
	//kore_websocket_send(c, 1, c->hdlr_extra,5);
	}
void oncandidate(struct rtcdc_peer_connection*peer,const char*candidate,void*user_data){
	printf("ON CANDIDATE &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&!%s\n",candidate);
	}
void websocket_message(struct connection *c, u_int8_t op, void *data, size_t len)
{
	if(data==NULL) return;
	
	
	fwrite((char*)data,1,len,stdout);
	printf("\n");
	//printf("DATA: %s\n",(const char*)data);
	json_t *root = load_json((const char*)data, len);
	if(root){
	 char*foo=json_dumps(root,0);
	kore_log(LOG_NOTICE,"incoming message: %s",foo);
		
	
	json_t *t=json_object_get(root,"type");
	const char*t_txt=json_string_value(t);
		if(!strcmp(t_txt,"message")){
		kore_log(LOG_NOTICE,"type message");
			//do_loop();
		}else if(!strcmp(t_txt,"login")){
		kore_log(LOG_NOTICE,"type login");
		}else{
		kore_log(LOG_NOTICE,"unknown type");
		}
kore_websocket_send(c,op,data,len);	
free(foo);
	}else{}

	json_decref(root);
}

void
websocket_disconnect(struct connection *c)
{
	kore_log(LOG_NOTICE, "%p: disconnecting", c);
}

void* helloworld(void*args){
printf("hello_world\n");
	   return 0;
}


int page(struct http_request *req)
{
	int status;
	int status_adr;
	pthread_t thread;
	status=pthread_create(&thread,NULL,helloworld,NULL);
	if(status !=0){http_response(req,500,"fuu",3);
		return (KORE_RESULT_OK);}
	status=pthread_join(thread,(void**)&status_adr);
	if(status !=0){http_response(req,500,"fuu",3);
		return (KORE_RESULT_OK);}
	kore_log(LOG_NOTICE,"status address %d",status_adr);
	
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

int page_ws_connect(struct http_request *req)
{
	//req->hdlr_extra=0;
	/* Perform the websocket handshake, passing our callbacks. */
	kore_log(LOG_NOTICE,"some path %s",req->path);
	kore_log(LOG_NOTICE, "%p: http_request", req);
	kore_websocket_handshake(req, "websocket_connect","websocket_message","websocket_disconnect");

	return (KORE_RESULT_OK);
}
gboolean cb(gpointer arg){
struct	kore_task *t=(struct kore_task*)arg;
g_print("hello world\n");
g_message("msg");
kore_task_channel_write(t,"channel\0",8);
	//fflush();
if(--count==0){
g_print("g main loop quit\n");
	printf("zifr %d\n",count);
g_main_loop_quit(loop);
	//count=10;
return FALSE;
}
return TRUE;
}
//int do_loop(void){

int pipe_reader(struct kore_task *t){
	kore_log(LOG_NOTICE,"fuuuuuuuuuuuuuuuuuck");
	do_loop(t);
	kore_task_channel_write(t,"papa\0",5);
	g_print("g main loop unref\n");
	
	return (KORE_RESULT_OK);
}
void pipe_data_available(struct kore_task *t){
	size_t len;
	u_int8_t buf[BUFSIZ];
if(kore_task_finished(t)){
kore_log(LOG_NOTICE,"bla bla bla");
return;
}
	len=kore_task_channel_read(t,buf,sizeof(buf));
	//if(len > buf){printf("len great than buf\n");}
	kore_log(LOG_NOTICE,"task msg: %s",buf);
}
//int pipe_reader(kore_task*t){}
int do_loop(struct kore_task *t){
g_print("g main loop new\n");
	loop=g_main_loop_new(NULL,FALSE);
	g_timeout_add(100,cb,t);
	g_print("g main loop run\n");
	g_main_loop_run(loop);
	g_print("g main loop unref\n");
	g_main_loop_unref(loop);
	return 0;
}

int rtc_loop(struct kore_task*t){
int dc_open=0;
	struct rtcdc_peer_connection*rtcdc_pc; 
	void onmessage(struct rtcdc_data_channel*channel,int datatype,void*data,size_t len,void*user_data){
	printf("\n data  received %s\n",(char*)data);
	}
	void onopen(struct rtcdc_data_channel*channel,void*user_data){
	printf("\n data channel opened\n");
		dc_open=1;
	}
	void onclose(struct rtcdc_data_channel*channel,void*user_data){
	printf("\ndata channel closed\n");
		dc_open=0;
	}
	void onconnect(struct rtcdc_peer_connection*peer,void*user_data){
	printf("\npeer connection established\n");
		rtcdc_create_data_channel(peer,"test-dc","",onopen,onmessage,onclose,user_data);
	}
	void onchannel(struct rtcdc_peer_connection*peer,struct rtcdc_data_channel*channel,void *user_data){
	printf("\nchannel created %s\n",channel->label);
		channel->on_message=onmessage;
	}
	/*
	void oncandidate(struct rtcdc_peer_connection*peer,const char*candidate,void*user_data){
	printf("ON CANDIDATE!\n");
	}
	*/
	//void*user_data;
	
	printf("\ncreating peer connection\n");
	//rtcdc_pc=rtcdc_create_peer_connection(onchannel,oncandidate,onconnect,"stun.services.mozilla.com",3478,user_data);
	rtcdc_pc=rtcdc_create_peer_connection(onchannel,oncandidate,onconnect,NULL,3478,/*user_data*/NULL);

	char*offer=rtcdc_generate_offer_sdp(rtcdc_pc);
	char*lcsdp=rtcdc_generate_local_candidate_sdp(rtcdc_pc);
	printf("offer no encode %s\n",offer);
	printf("lcsdp: %s\n",lcsdp);
	gchar*b_offer=g_base64_encode((const guchar*)offer,strlen(offer));
	printf("\noffer sdp: %s\n",b_offer);
	gchar*b_lcsdp=g_base64_encode(lcsdp,strlen(lcsdp));
	printf("\n Local candidate: %s\n",b_lcsdp);
	sleep(3);
	
	//char*lcsdp2=rtcdc_generate_local_candidate_sdp(per);
	//gchar*b_lcsdp2=g_base64_encode(lcsdp2,strlen(lcsdp2));
	//
	
	
	
	
	
	gsize dec_remote_sdp_len=0;
	printf("\nenter remote sdp twice: \n");
	//const gchar*remote_sdp_offer=getlines();
	const char*dec_remote_sdp_offer=g_base64_decode(b_offer,&dec_remote_sdp_len);
	int parse_offer=rtcdc_parse_offer_sdp(rtcdc_pc,dec_remote_sdp_offer);
	if(parse_offer >=0){
	offer=rtcdc_generate_offer_sdp(rtcdc_pc);
	b_offer=g_base64_encode((const guchar*)offer,strlen(offer));
		printf("\n new offer sdp %s\n",b_offer);
	}else{
	printf("\n invalid sdp offer %d\n",parse_offer);
		_exit(1);
	}
	printf("\n enter remote candidate: \n");
	const gchar*remote_candidate=b_lcsdp;//getlines();
	gsize dec_candidate_len;
	 gchar*dec_remote_candidate=g_base64_decode(remote_candidate,&dec_candidate_len);
	int parse_candidate=rtcdc_parse_candidate_sdp(rtcdc_pc,dec_remote_candidate);
	if(parse_candidate>0){
		printf("valid candidate\n");
	}else{
	printf("invalid candidate\n");
		//_exit(1);
		//return (KORE_RESULT_ERROR);
	}
	//pthread_t tid;int status_adr;
	//pthread_create(&tid,NULL,rtcdc_e_loop,(void*)rtcdc_pc);
	rtcdc_loop(rtcdc_pc);
	printf("HERE1\n");
	struct rtcdc_data_channel*channel=NULL;
	while(1==1){
	if(rtcdc_pc->initialized>0){
		//printf("HERE2=%d\n",dc_open);
		//printf("DC_OPEN : %d\n",dc_open);
	
		if(dc_open==1){
			printf("HERE3\n");
		channel=rtcdc_pc->channels[0];
			if(channel->state > RTCDC_CHANNEL_STATE_CLOSED){
			printf("enter a message\n");
				gchar*message="fuuuu channel\0";//getlines();
				rtcdc_send_message(channel,RTCDC_DATATYPE_STRING,message,strlen(message));
				printf("message sent\n");
				g_free(message);
			}
		}
	}
		sleep(1);
	}
	//pthread_join(tid,(void**)&status_adr);
	g_free((gpointer*)dec_remote_sdp_offer);
	g_free((void*)dec_remote_candidate);
return (KORE_RESULT_OK);
}
