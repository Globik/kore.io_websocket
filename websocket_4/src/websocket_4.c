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

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
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

static void *rtcdc_e_loop(void*);
static void onmessage(struct rtcdc_data_channel*,int, void*,size_t,void*);
static void onopen(struct rtcdc_data_channel*,void*);
static void onclose(struct rtcdc_data_channel*,void*);
static void onconnect(struct rtcdc_peer_connection*,void*);
static void onchannel(struct rtcdc_peer_connection*,struct rtcdc_data_channel*,void *);
static void on_candidate(struct rtcdc_peer_connection*, const char*, void*);
void handle_candidate(char*);
void handle_offer(json_t*, struct connection*);
void handle_answer(json_t*);
void create_pc(struct connection*);

int dc_open=0;
struct rtcdc_peer_connection *bob=NULL;

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
	json_t *reply=json_object();
	json_object_set_new(reply,"type",json_string("message"));
	
	json_object_set_new(reply,"msg",json_string("Hallo jason!"));
	size_t size=json_dumpb(reply,NULL,0,0);
	if(size==0){printf("Size is null\n");}
	char*buf=alloca(size);
	size=json_dumpb(reply,buf,size,0);
	printf("buffer: %s\n",buf);
	kore_websocket_send(c, 1, buf,size);
	json_decref(reply);
	//free((charbuf);
	}

void websocket_message(struct connection *c, u_int8_t op, void *data, size_t len)
{
	if(data==NULL) return;
	//kore_log(LOG_NOTICE,"some message: %s",(char*)data);
	
	//fwrite((char*)data,1,len,stdout);
	//printf("\n");
	/*
	char*offer_sdp=(char*)data;
	
char*offer="v=0 \n"\
"o=- 2701145278607694824 2 IN IP4 127.0.0.1 \n" \
"s=- \n"              \
"t=0 0 \n"             \
"a=group:BUNDLE data \n" \
"a=msid-semantic: WMS \n" \
"m=application 9 DTLS/SCTP 5000 \n"\
"c=IN IP4 0.0.0.0 \n"\
"a=ice-ufrag:nGsD \n"\
"a=ice-pwd:hZPKMtow1DEM4fL7iUppDJHA\n"\
"a=ice-options:trickle \n"\
"a=fingerprint:sha-256 C1:50:3F:D4:C7:F8:E1:F6:FF:67:DA:73:40:5E:0C:89:76:C3:58:AF:23:F3:07:05:6E:02:2E:05:C6:2E:F8:4E "\
"a=setup:actpass \n"\
"a=mid:data \n"\
"a=sctpmap:5000 webrtc-datachannel 1024\n";
*/
	/*
	with internet connection
v=0
o=- 8848996438182133161 2 IN IP4 127.0.0.1
s=-
t=0 0
a=group:BUNDLE data
a=msid-semantic: WMS
m=application 9 DTLS/SCTP 5000
c=IN IP4 0.0.0.0
a=ice-ufrag:LV9g
a=ice-pwd:CoCdNISUrqZTDOLqW8T3uXsh
a=ice-options:trickle
a=fingerprint:sha-256 87:A3:AC:7B:60:38:77:EA:55:5C:89:F9:85:E4:47:9D:BD:5B:BA:CC:E0:3E:3E:53:1C:BC:E1:20:B0:9B:EA:2D
a=setup:actpass
a=mid:data
a=sctpmap:5000 webrtc-datachannel 1024
a=candidate:1047372208 1 udp 2113937151 10.34.73.56 49670 typ host generation 0 ufrag LV9g network-cost 50
*/
/*	
v=0
o=- 5663417401290824 2 IN IP4 127.0.0.1
s=-
t=0 0
a=msid-semantic: WMS
m=application 1 UDP/DTLS/SCTP webrtc-datachannel
c=IN IP4 0.0.0.0
a=ice-ufrag:5Ld6
a=ice-pwd:m9qMJjGpa0mpbfd0uzQYY5
a=fingerprint:sha-256 EA
a=setup:active
a=mid:data
a=sctp-port:45699
*/	/*
	printf("OFFERRRRRRRRRRR!: %s\n",offer);
	
	
	int a=rtcdc_parse_offer_sdp(bob, offer_sdp);
	if(a >= 0){
	printf(green "parse offer by Bob OK = %d\n" rst, a);
	}else{
	printf(red "parse offer by Bob NOT OK = %d\n" rst, a);
	//_exit(1);
	return;
	}
	*/
	//char*remote_cand_sdp=rtcdc_generate_local_candidate_sdp(bob);
	//kore_websocket_send(c, 1, remote_cand_sdp,strlen(remote_cand_sdp));
	//char*remote_offer_sdp
		//offer_sdp=rtcdc_generate_offer_sdp(bob);
	//char*remote_cand_sdp=rtcdc_generate_local_candidate_sdp(bob);
	//printf("CANDIDATE ON SERVER: \n %s\n",remote_cand_sdp);
	//printf("REMOTE OFFER ON SERVER: \n %s\n",remote_offer_sdp);
	/*
	int x = rtcdc_parse_candidate_sdp(bob, remote_cand_sdp);
	if(x > 0){
	printf(green "Remote Candidate OK by Bob = %d\n" rst, x);
	}else{
	printf("Remote Candidate NOT OK by Bob = %d\n", x);
	return;
	}
	*/
	/*
	int a=rtcdc_parse_offer_sdp(bob, offer_sdp);
	if(a >= 0){
	printf(green "parse offer by Bob OK = %d\n" rst, a);
	}else{
	printf(red "parse offer by Bob NOT OK = %d\n" rst, a);
	//_exit(1);
	return;
	}*/
	
	//kore_websocket_send(c, 1, offer_sdp,strlen(offer_sdp));
	

	
	//printf("DATA: %s\n",(const char*)data);
	
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
}

void
websocket_disconnect(struct connection *c)
{
	kore_log(LOG_NOTICE, "%p: disconnecting", c);
}

void handle_candidate(char*obj){
/*json_error_t error;
char*type_m;
 char*cand;
int aber=json_unpack_ex(obj,&error,JSON_STRICT,"{s:s,s:s}","type",&type_m,"cand",&cand);
	printf("is ok ab: %d\n",aber);
	if(aber==-1){
	printf("error: %s\n",error.text);
	printf("source: %s\n",error.source);
	printf("line: %d\n",error.line);
	printf("position: %d\n",error.position);
	printf("column: %d\n",error.column);
	return;
	}
	printf("type: %s\n",type_m);
	*/
	printf("candidate: %s\n", obj);
	int x = rtcdc_parse_candidate_sdp(bob, obj);
	if(x > 0){
	printf(green "Remote Candidate OK by Bob = %d\n" rst, x);
	}else{
	printf("Remote Candidate NOT OK by Bob = %d\n", x);
	return;
	}
}

void create_pc(struct connection*c){
	/*
	bob=rtcdc_create_peer_connection(onchannel,on_candidate,onconnect,
										"stun.services.mozilla.com",3478, c);
	GThread *boba=g_thread_new("tobob", &rtcdc_e_loop,(void*)bob);
	g_thread_join(boba);
   g_thread_unref(boba);*/
//return 0;
	//.....
}

void handle_offer(json_t*obj,struct connection*c){
json_error_t error;
char*type_m;
 char*remote_offer_sdp;
int aber=json_unpack_ex(obj,&error,JSON_STRICT,"{s:s,s:s}","type",&type_m,"sdp",&remote_offer_sdp);
	printf("is ok aber from handle_offer: %d\n",aber);
	if(aber==-1){
	printf("error: %s\n",error.text);
	printf("source: %s\n",error.source);
	printf("line: %d\n",error.line);
	printf("position: %d\n",error.position);
	printf("column: %d\n",error.column);
	return;
	}
	//create_pc(c);jiji
	int y = rtcdc_parse_offer_sdp(bob, remote_offer_sdp);
	if(y >= 0){
	printf(green "Parse offer by Bob OK = %d\n" rst, y);
	}else{
	printf(red "Parse offer by Bob NOT OK = %d\n" rst, y);
	return;
	}
	char*suka=rtcdc_generate_local_candidate_sdp(bob);
	printf("SUKA: %s\n",suka);
	char*sdp=rtcdc_generate_offer_sdp(bob);
	//{type:answer,answer:remote_offer_sdp}
	json_t *reply=json_object();
	json_object_set_new(reply,"type",json_string("answer"));
	//json_object_set_new(reply,"session_id",json_integer(5));
	json_object_set_new(reply,"answer",json_string(sdp));
  // const char*line=json_dumps(reply,0);
	//size_t siz = json_object_size(reply);
	size_t size=json_dumpb(reply,NULL,0,0);
	if(size==0){printf("Size is null\n");return;}
	char*buf=alloca(size);
	size=json_dumpb(reply,buf,size,0);
	printf("buffer: %s\n",buf);
	kore_websocket_send(c, 1, buf,size);
	json_decref(reply);	
	
	char*remote_cand_sdp=rtcdc_generate_local_candidate_sdp(bob);
	printf(red"BOB: CAND_SDP: \n %s\n"rst,remote_cand_sdp);
	
	json_t *reply2=json_object();
	json_object_set_new(reply2,"type",json_string("candidate"));
	//json_object_set_new(reply,"session_id",json_integer(5));
	json_object_set_new(reply2,"cand",json_string(remote_cand_sdp));
  // const char*line=json_dumps(reply,0);
	//size_t siz = json_object_size(reply);
	size_t size2=json_dumpb(reply2,NULL,0,0);
	if(size==0){printf("Size is null\n");return;}
	char*buf2=alloca(size2);
	size=json_dumpb(reply2,buf2,size2,0);
	//printf("buffer: %s\n",buf);
	kore_websocket_send(c, 1, buf2,size2);
	json_decref(reply2);	
	//json_decref(reply);
}

void handle_answer(json_t*obj){
json_error_t error;
char*type_m;
 char*answer;
int aber=json_unpack_ex(obj,&error,JSON_STRICT,"{s:s,s:s}","type",&type_m,"answer",&answer);
	printf("is ok aber: %d\n",aber);
	if(aber==-1){
	printf("error: %s\n",error.text);
	printf("source: %s\n",error.source);
	printf("line: %d\n",error.line);
	printf("position: %d\n",error.position);
	printf("column: %d\n",error.column);
	return;
	}
	int y = rtcdc_parse_offer_sdp(bob, answer);
	if(y >= 0){
	printf(green "Parse answer by Bob OK = %d\n" rst, y);
	}else{
	printf(red "Parse answer by Bob NOT OK = %d\n" rst, y);
	return;
	}
	
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

void pipe_data_available(struct kore_task *t){
	size_t len;
	u_int8_t buf[BUFSIZ];
	/*
if(kore_task_finished(t)){
kore_log(LOG_NOTICE,"Task finished.");
return;
}
*/
	len=kore_task_channel_read(t,buf,sizeof(buf));
	if(len > buf){printf("len great than buf\n");}
	kore_log(LOG_NOTICE,"TTTTTTTTTTTTTTTTTTTTTTTTTTTTTask msg: %s",buf);
}

static void *rtcdc_e_loop(void*peer){
struct rtcdc_peer_connection*p=(struct rtcdc_peer_connection*)peer;
printf("rtcdc_loop started.\n");
rtcdc_loop(p);
//rtcdc_destroy_peer_connection(p);
}


int rtc_loop(struct kore_task*t){
	
printf("\n Creating peer connection, Alice.\n");
//void*user_data;
GMainContext *context=NULL;
// struct rtcdc_peer_connection *
bob=rtcdc_create_peer_connection(onchannel,on_candidate,onconnect,"stun.services.mozilla.com",3478, t);
GMainLoop *mainloop=g_main_loop_new(context,FALSE);
GThread *abob=g_thread_new("tbob", &rtcdc_e_loop,(void*)bob);
g_main_loop_run(mainloop);
g_thread_join(abob);
g_thread_unref(abob);
g_main_loop_unref(mainloop);

return (KORE_RESULT_OK);
}

/*

void onmessage(struct rtcdc_data_channel*,int, void*,size_t,void*);
void onopen(struct rtcdc_data_channel*,void*);
void onclose(struct rtcdc_data_channel*,void*);
void onconnect(struct rtcdc_peer_connection*,void*);
void onchannel(struct rtcdc_peer_connection*,struct rtcdc_data_channel*,void *);
void on_candidate(struct rtcdc_peer_connection*, const char*, void*);

*/

void onmessage(struct rtcdc_data_channel*channel,int datatype,void*data,size_t len,void*user_data){
	printf(red "\n Data  received => %s\n" rst,(char*)data);
	if(channel->state > RTCDC_CHANNEL_STATE_CLOSED){
	char*message2="Hi! Wow. On_message.\0";
    rtcdc_send_message(channel,RTCDC_DATATYPE_STRING,message2,strlen(message2)+1); 
	}
	}
	void onopen(struct rtcdc_data_channel*channel,void*user_data){
	printf(green "\n Data channel opened!\n" rst);
	dc_open=1;
	if(channel->state > RTCDC_CHANNEL_STATE_CLOSED){
	char*message="Hi! I'm Bob. On_open.\0";
    rtcdc_send_message(channel,RTCDC_DATATYPE_STRING,message,strlen(message)+1); 
	}
	}
	void onclose(struct rtcdc_data_channel*channel,void*user_data){
	printf("\nData channel closed!\n");
	dc_open=0;
	}
	void onconnect(struct rtcdc_peer_connection*peer,void*user_data){
	printf(green "\nPeer connection established!\n" rst);
	printf("peer->role: %d\n",peer->role);
	rtcdc_create_data_channel(peer,"test-dc","",onopen,onmessage,onclose,user_data);
	}
	void onchannel(struct rtcdc_peer_connection*peer,struct rtcdc_data_channel*channel,void *user_data){
	printf("\nChannel created! With a channel->label: %s\n",channel->label);
	channel->on_message=onmessage;
	}
	void on_candidate(struct rtcdc_peer_connection*peer,const char*candidate,void*user_data){
	
	struct kore_task*t=(struct kore_task*)user_data;
		printf(yellow "ON CANDIDATE SUKA! %s\n" rst, candidate);
	//json_t *reply=json_object();
	//json_object_set_new(reply,"type",json_string("candidate"));
	//json_object_set_new(reply,"session_id",json_integer(5));
	//json_object_set_new(reply,"cand",json_string(candidate));
  // const char*line=json_dumps(reply,0);
	//size_t siz = json_object_size(reply);
	kore_task_channel_write(t,(void*)candidate,100);
	//kore_websocket_send(c,1,reply, siz);
	//json_decref(reply);
	}
