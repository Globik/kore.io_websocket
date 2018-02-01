/*
https://github.com/xhs/librtcdc
*/
#include <stdio.h>
#include "src/rtcdc.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <pthread.h>
#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
static GMainLoop *mainloop;
static void free_all(struct rtcdc_data_channel*);
void *rtcdc_e_loop(void*peer){
struct rtcdc_peer_connection*p=(struct rtcdc_peer_connection*)peer;
	printf("MOOOOOOOOOOO\n");
rtcdc_loop(p);
	printf("dooooooooooooooo\n");
	rtcdc_destroy_peer_connection(p);
}

struct rtcdc_data_channel *channel, *channel2;
int main(){
struct rtcdc_peer_connection*alice, *bob;
GMainContext *context=NULL;
	GSource *source_alice,*source_bob;
	
int dc_open=0;
	
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
	rtcdc_create_data_channel(peer,"test-dc","",onopen,onmessage,onclose,user_data);
	}
	void onchannel(struct rtcdc_peer_connection*peer,struct rtcdc_data_channel*channel,void *user_data){
	printf("\nChannel created! With a channel->label: %s\n",channel->label);
	channel->on_message=onmessage;
		//rtcdc_destroy_peer_connection(peer);
	}
	void on_candidate(struct rtcdc_peer_connection*peer,const char*candidate,void*user_data){
	printf(yellow "ON CANDIDATE! %s\n" rst, candidate);
	}
	void*user_data;
	
printf(green "\n Creating peer connection, Bob and Alice.\n" rst);
//struct rtcdc_peer_connection*
	alice=rtcdc_create_peer_connection(onchannel,on_candidate,onconnect,"stun.services.mozilla.com",3478, user_data);

//struct rtcdc_peer_connection*
	bob=rtcdc_create_peer_connection(onchannel,on_candidate,onconnect,"stun.services.mozilla.com",3478,user_data);
	mainloop=g_main_loop_new(context,FALSE);
	//g_source_attach(source_alice,context);
	//g_source_attach(source_bob,context);
	
	GThread *al=g_thread_new("talice", &rtcdc_e_loop,(void*)alice);
	GThread *bo=g_thread_new("tbob", &rtcdc_e_loop,(void*)bob);
	
	char*offer_sdp=rtcdc_generate_offer_sdp(alice);
	char*local_cand_sdp=rtcdc_generate_local_candidate_sdp(alice);
	printf(yellow "offer_sdp:\n %s\n" rst,offer_sdp);
	printf("local_cand_sdp:\n %s\n",local_cand_sdp);
	
	int a=rtcdc_parse_offer_sdp(bob, offer_sdp);
	if(a >= 0){
	printf(green "parse offer by Bob OK = %d\n" rst, a);
	}else{
	printf(red "parse offer by Bob NOT OK = %d\n" rst, a);
	_exit(1);
	}
	char*remote_offer_sdp=rtcdc_generate_offer_sdp(bob);
	
	char*remote_cand_sdp=rtcdc_generate_local_candidate_sdp(bob);
	
	int y = rtcdc_parse_offer_sdp(alice, remote_offer_sdp);
	if(y >= 0){
	printf(green "Parse offer by Alice OK = %d\n" rst, y);
	}else{
	printf(red "Parse offer by Alice NOT OK = %d\n" rst, y);
	_exit(1);
	}
	
	int pr = rtcdc_parse_candidate_sdp(alice, remote_cand_sdp);
	if(pr > 0){
	printf(green "Remote Candidate sdp by Alice OK = %d\n" rst, pr);
	}else{
	printf("Remote Candidate by Alice NOT OK = %d\n", pr);
	_exit(1);
	}
	int x = rtcdc_parse_candidate_sdp(bob, local_cand_sdp);
	if(x > 0){
	printf(green "Remote Candidate OK by Bob = %d\n" rst, x);
	}else{
	printf("Remote Candidate NOT OK by Bob = %d\n", x);
	_exit(1);
	}
	
g_main_loop_run(mainloop);
	g_thread_join(al);
	g_thread_join(bo);
	g_thread_unref(al);
	g_thread_unref(bo);
	g_main_loop_unref(mainloop);

return EXIT_SUCCESS;
}

/*	

offer no encode 
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
// candidate
lcsdp: a=candidate:1 1 UDP 2013266431 10.34.42.237 60618 typ host
a=candidate:2 1 TCP 1019216383 10.34.42.237 9 typ host tcptype active
a=candidate:3 1 TCP 1015022079 10.34.42.237 56045 typ host tcptype passive
*/