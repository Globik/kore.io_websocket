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
//include <glib.h>
#include <pthread.h>
#include <jansson.h>
#include "assets.h"




#include "Microstack/ILibParsers.h"
#include "Microstack/ILibAsyncSocket.h"
#include "Microstack/ILibWebRTC.h"
#include "core/utils.h"
#include "Microstack/ILibWrapperWebRTC.h"
#include "SimpleRendezvousServer.h"


#define WEBSOCKET_PAYLOAD_SINGLE	125
#define WEBSOCKET_PAYLOAD_EXTEND_1	126
#define WEBSOCKET_PAYLOAD_EXTEND_2	127

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
//struct rstate{struct kore_task task;};
//char *stunServerList[] = { "stun.ekiga.net", "stun.ideasip.com", "stun.schlund.de", "stunserver.org", "stun.softjoys.com", "stun.voiparound.com", "stun.voipbuster.com", "stun.voipstunt.com", "stun.voxgratia.org" };
int useStun = 0;

ILibWrapper_WebRTC_ConnectionFactory mConnectionFactory;
ILibWrapper_WebRTC_Connection mConnection;
ILibWrapper_WebRTC_DataChannel *mDataChannel = NULL;
SimpleRendezvousServer mServer;

void* chain;
char *stunServerList[] = { "stun.ekiga.net", "stun.ideasip.com", "stun.schlund.de", "stunserver.org", "stun.softjoys.com", "stun.voiparound.com", "stun.voipbuster.com", "stun.voipstunt.com", "stun.voxgratia.org" };
//int useStun = 0;
void OnDataChannelData(ILibWrapper_WebRTC_DataChannel *,char *,int);
void OnDataChannelClosed(ILibWrapper_WebRTC_DataChannel *);
void OnDataChannelAck(ILibWrapper_WebRTC_DataChannel *);
void WebRTCConnectionSink(ILibWrapper_WebRTC_Connection, int);
void WebRTCDataChannelSink(ILibWrapper_WebRTC_Connection, ILibWrapper_WebRTC_DataChannel *);
void WebRTCConnectionSendOkSink(ILibWrapper_WebRTC_Connection);


void OnDataChannelData(ILibWrapper_WebRTC_DataChannel *dataChannel,char *buffer,int bufferLen){
buffer[bufferLen] = 0;
printf("Received data on [%s]: %s\r\n", dataChannel->channelName, buffer);
}
void OnDataChannelClosed(ILibWrapper_WebRTC_DataChannel *dataChannel)
{
	printf("DataChannel [%s]:%u was closed\r\n", dataChannel->channelName, dataChannel->streamId);
}
void OnDataChannelAck(ILibWrapper_WebRTC_DataChannel *dataChannel)
{
	mDataChannel = dataChannel;
	printf("DataChannel [%s] was successfully ACK'ed\r\n", dataChannel->channelName);
	mDataChannel->OnStringData = (ILibWrapper_WebRTC_DataChannel_OnData)&OnDataChannelData;
	mDataChannel->OnClosed = (ILibWrapper_WebRTC_DataChannel_OnClosed)&OnDataChannelClosed;
}

void WebRTCConnectionSink(ILibWrapper_WebRTC_Connection connection, int connected)
{
	if(connected)
	{
		printf("WebRTC connection Established. [%s]\r\n", ILibWrapper_WebRTC_Connection_DoesPeerSupportUnreliableMode(connection)==0?"RELIABLE Only":"UNRELIABLE Supported");
		ILibWrapper_WebRTC_DataChannel_Create(connection, "MyDataChannel", 13, &OnDataChannelAck);
	}
	else
	{
		printf("WebRTC connection is closed.\r\n");
		mConnection = NULL;
		mDataChannel = NULL;
	}
}

// This is called when the remote side created a data channel

void WebRTCDataChannelSink(ILibWrapper_WebRTC_Connection connection, ILibWrapper_WebRTC_DataChannel *dataChannel)
{
	printf("WebRTC Data Channel (%u:%s) was created.\r\n", dataChannel->streamId, dataChannel->channelName);
	mDataChannel = dataChannel;
	mDataChannel->OnStringData = (ILibWrapper_WebRTC_DataChannel_OnData)&OnDataChannelData;
	mDataChannel->OnClosed = (ILibWrapper_WebRTC_DataChannel_OnClosed)&OnDataChannelClosed;
}

void WebRTCConnectionSendOkSink(ILibWrapper_WebRTC_Connection connection)
{
	UNREFERENCED_PARAMETER(connection);
}

/*
void OnWebSocket(SimpleRendezvousServerToken sender, int InterruptFlag, struct packetheader *header, char *bodyBuffer,
				 int bodyBufferLen, SimpleRendezvousServer_WebSocket_DataTypes bodyBufferType, SimpleRendezvousServer_DoneFlag done)
{	
	printf(red"on websocket\n"rst);
	
	if(done == SimpleRendezvousServer_DoneFlag_NotDone)
	{
		
		
		
		
		
		
		
		
		
		printf(yellow"We have the entire offer\n"rst);
			char *offer;
		if (mConnection == NULL)
		{
			printf(yellow"The browser initiated the SDP offer, so we have to create a connection and set the offer\n"rst);
			mConnection = ILibWrapper_WebRTC_ConnectionFactory_CreateConnection(mConnectionFactory, 
																				&WebRTCConnectionSink, 
																				&WebRTCDataChannelSink, 
																				&WebRTCConnectionSendOkSink);
			ILibWrapper_WebRTC_Connection_SetStunServers(mConnection, stunServerList, 9);

			if (useStun==0)
			{
				printf(red"stun is 0\n"rst);
				offer = ILibWrapper_WebRTC_Connection_SetOffer(mConnection, bodyBuffer, bodyBufferLen, NULL);
				SimpleRendezvousServer_WebSocket_Send(sender, SimpleRendezvousServer_WebSocket_DataType_TEXT, offer, strlen(offer), ILibAsyncSocket_MemoryOwnership_CHAIN, SimpleRendezvousServer_FragmentFlag_Complete);
			}
			else
			{
				printf(yellow"We're freeing this, becuase we'll generate the offer in the candidate callback...\n"rst);
				// The best way, is to return this offer, and update the candidate incrementally, but that is for another sample
				ILibWrapper_WebRTC_Connection_SetUserData(mConnection, NULL, sender, NULL);
				free(ILibWrapper_WebRTC_Connection_SetOffer(mConnection, bodyBuffer, bodyBufferLen, &CandidateSink));
			}
		}
		else
		{
			printf(yellow"We inititiated the SDP exchange, so the browser is just giving us a response... Even tho, this will generate a counter-response\n"rst);
			// we don't need to send it back to the browser, so we'll just drop it.
			printf(red"Setting Offer...\r\n"rst);
			free(ILibWrapper_WebRTC_Connection_SetOffer(mConnection, bodyBuffer, bodyBufferLen, NULL));	
		}
	
	
	
	
	
	
	
	
	}
}
*/

//#if defined(_POSIX)
void BreakSink(int s)
{
	UNREFERENCED_PARAMETER( s );

	signal(SIGINT, SIG_IGN);	// To ignore any more ctrl c interrupts
	
	ILibStopChain(chain); // Shutdown the Chain
}
//#endif
struct kore_task pipe_task;

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
	
	fwrite((char*)data,1,len,stdout);
	printf("\n");

	printf(yellow"We have the entire offer\n"rst);
			char *offer;
		if (mConnection == NULL)
		{
			printf(yellow"The browser initiated the SDP offer, so we have to create a connection and set the offer\n"rst);
			mConnection = ILibWrapper_WebRTC_ConnectionFactory_CreateConnection(mConnectionFactory, 
																				&WebRTCConnectionSink, 
																				&WebRTCDataChannelSink, 
																				&WebRTCConnectionSendOkSink);
			//ILibWrapper_WebRTC_Connection_SetStunServers(mConnection, stunServerList, 9);

			if (useStun==0)
			{
				printf(red"stun is 0\n"rst);
				offer = ILibWrapper_WebRTC_Connection_SetOffer(mConnection, data, len, NULL);
				printf(red"Offer:\n %s\n"rst,offer);
				//SimpleRendezvousServer_WebSocket_Send(sender, SimpleRendezvousServer_WebSocket_DataType_TEXT, offer, 
				//strlen(offer), ILibAsyncSocket_MemoryOwnership_CHAIN, SimpleRendezvousServer_FragmentFlag_Complete);
				kore_websocket_send(c, 1, offer,strlen(offer));
			}
			else
			{
				//printf(yellow"We're freeing this, becuase we'll generate the offer in the candidate callback...\n"rst);
				// The best way, is to return this offer, and update the candidate incrementally, but that is for another sample
				//ILibWrapper_WebRTC_Connection_SetUserData(mConnection, NULL, sender, NULL);
				//free(ILibWrapper_WebRTC_Connection_SetOffer(mConnection, bodyBuffer, bodyBufferLen, &CandidateSink));
			}
		}
		else
		{
			printf(yellow"We inititiated the SDP exchange, so the browser is just giving us a response... Even tho, this will generate a counter-response\n"rst);
			// we don't need to send it back to the browser, so we'll just drop it.
			printf(red"Setting Offer...\r\n"rst);
			//free(ILibWrapper_WebRTC_Connection_SetOffer(mConnection, bodyBuffer, bodyBufferLen, NULL));	
		}
	
	
	
	
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

void Run()
{
	char temp[1024];
	char* line;

	while(ILibIsChainBeingDestroyed(chain)==0)
	{
		line = fgets(temp, 1024, stdin);

		if (mDataChannel != NULL && line!=NULL)
		{
			ILibWrapper_WebRTC_DataChannel_Close(mDataChannel);
			//ILibWrapper_WebRTC_DataChannel_SendString(mDataChannel, line, strlen(line)); // Send string data over the WebRTC Data Channel
		}
	}
}


int rtc_loop(struct kore_task*t){
/*	
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
*/
	
	signal(SIGPIPE, SIG_IGN); // Set a SIGNAL on Linux to listen for Ctrl-C

	// Shutdown on Ctrl + C
	signal(SIGINT, BreakSink);
	{
		struct sigaction act; 
		act.sa_handler = SIG_IGN; 
		sigemptyset(&act.sa_mask); 
		act.sa_flags = 0; 
		sigaction(SIGPIPE, &act, NULL);
	}
	
	chain = ILibCreateChain();	// Create the MicrostackChain, to which we'll attach the WebRTC ConnectionFactory
	mConnectionFactory = ILibWrapper_WebRTC_ConnectionFactory_CreateConnectionFactory(chain, 0); 
	ILibSpawnNormalThread(&Run, NULL); // Spawn a thread to listen for user input
	ILibStartChain(chain); 
	
	
	printf("Application exited gracefully.\r\n");
	
return (KORE_RESULT_OK);
}


/*
librtcdc
offer_sdp BY ALICE:
v=0
o=- 8390432355997150 2 IN IP4 127.0.0.1
s=-
t=0 0
a=msid-semantic: WMS
m=application 1 UDP/DTLS/SCTP webrtc-datachannel
c=IN IP4 0.0.0.0
a=ice-ufrag:lNU4
a=ice-pwd:wNSmpiWmfPyF8YvF0AL8ye
a=fingerprint:sha-256 36
a=setup:active
a=mid:data
a=sctp-port:14057

answer_SDP by bob: 
v=0
o=- 8390432355997150 2 IN IP4 127.0.0.1
s=-
t=0 0
a=msid-semantic: WMS
m=application 1 UDP/DTLS/SCTP webrtc-datachannel
c=IN IP4 0.0.0.0
a=ice-ufrag:PBo9
a=ice-pwd:IUC+lNASGnZlpoiWX4SCIO
a=fingerprint:sha-256 36
a=setup:passive
a=mid:data
a=sctp-port:14057

CHROME OFFER TO MESHCENTRAL:

	v=0 
	o=- 2084041186933435364 2 IN IP4 127.0.0.1 
	s=- 
	t=0 0 
	a=group:BUNDLE data
	a=msid-semantic: WMS 
	m=application 9 DTLS/SCTP 5000 
	c=IN IP4 0.0.0.0 
	a=ice-ufrag:HskY 
	a=ice-pwd:37A2Vb0Xt/EzSOCYug9M4ztI 
	a=ice-options:trickle 
	a=fingerprint:sha-256 9C:38:95:FC:46:7F:FD:F0:77:A3:8C:62:24:ED:77:E7:55:22:7F:F1:42:92:91:2C:75:C8:E6:62:69:31:9A:D5 
	a=setup:actpass 
	a=mid:data 
	a=sctpmap:5000 webrtc-datachannel 1024 
	a=candidate:3537527976 1 udp 2113937151 10.34.49.165 59307 typ host generation 0 ufrag HskY network-cost 50 

	
MESHCENTRAL ANSWER
v=0
o=MeshAgent 59198 0 IN IP4 0.0.0.0
s=SIP Call
t=0 0
a=ice-ufrag:A781A0BA
a=ice-pwd:13D57096A3D6A33017E51BA37178402A
a=fingerprint:sha-256 A0:80:0B:ED:AB:3C:F3:37:C5:14:57:AF:9D:31:88:43:CA:C0:BD:29:67:9C:57:8F:E9:55:E4:63:10:31:92:89
m=application 1 DTLS/SCTP 5000
c=IN IP4 0.0.0.0
a=sctpmap:5000 webrtc-datachannel 16
a=setup:passive
a=candidate:0 1 UDP 2128609535 10.34.49.165 57159 typ host
a=candidate:0 2 UDP 2128609535 10.34.49.165 57159 typ host
*/