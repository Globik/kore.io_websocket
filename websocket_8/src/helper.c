//#include <kore/kore.h>
//#include <kore/http.h>
//#include <kore/tasks.h>
/*
gcc `pkg-config --cflags glib-2.0` -c helper.c
# ar -cvq libhelper.a  helper.o

*/


#include "helper.h"

#define WEBSOCKET_PAYLOAD_SINGLE	125
#define WEBSOCKET_PAYLOAD_EXTEND_1	126
#define WEBSOCKET_PAYLOAD_EXTEND_2	127

static void websocket_frame_build(struct kore_buf *, u_int8_t, const void *,size_t);
static gboolean j_check_sess(gpointer);

void kore_websocket_broadcast_room(struct connection *src, u_int8_t op, const void *data, size_t len, int scope)
{
	struct connection	*c;
	struct kore_buf		*frame;

	frame = kore_buf_alloc(len);
	websocket_frame_build(frame, op, data, len);

	TAILQ_FOREACH(c, &connections, list) {
		if (c->hdlr_extra==src->hdlr_extra && c->proto == CONN_PROTO_WEBSOCKET) {
			if(c->hdlr_extra !=NULL){
				if(c->hdlr_extra==src->hdlr_extra){
			net_send_queue(c, frame->data, frame->offset);
			net_send_flush(c);
				}
			}
		}
	}

	if (scope == WEBSOCKET_BROADCAST_GLOBAL) {
		kore_msg_send(KORE_MSG_WORKER_ALL,
		    KORE_MSG_WEBSOCKET, frame->data, frame->offset);
	}

	kore_buf_free(frame);
}
void kore_websocket_broadcast_room_char(const char*src, u_int8_t op, const void *data, size_t len, int scope)
{
	
	struct connection	*c;
	struct kore_buf		*frame;

	frame = kore_buf_alloc(len);
	websocket_frame_build(frame, op, data, len);
	if(worker->active_hdlr==NULL){
	kore_log(LOG_INFO,"worker->active_hdlr is NULL! id: %d",worker->id);
	}else{
	kore_log(LOG_INFO,"worker->active_hdlr is NOT NULL!");
	}
if(src !=NULL){kore_log(LOG_INFO,"some src in broadcast : %s",(const char*)src);}else{
kore_log(LOG_INFO,"src is undefined");
}
	
	TAILQ_FOREACH(c, &connections, list) {
		if (c->proto == CONN_PROTO_WEBSOCKET) {
			if(c->hdlr_extra !=NULL){
				 if(c->hdlr_extra==src ){
			net_send_queue(c, frame->data, frame->offset);
			net_send_flush(c);
				 }}
		}
	}
	
	if (scope == WEBSOCKET_BROADCAST_GLOBAL) {
	kore_msg_send(KORE_MSG_WORKER_ALL,KORE_MSG_WEBSOCKET, frame->data, frame->offset);
	}

	kore_buf_free(frame);
}


static void websocket_frame_build(struct kore_buf *frame, u_int8_t op, const void *data,size_t len)
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

 //gint stop_signal=0;
 //volatile gint stop=0;
void j_handle_signal(int signum) {
	stop_signal = signum;
	switch(g_atomic_int_get(&stop)) {
		case 0:
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

static gboolean j_check_sess(gpointer user_data){
	//kore_log(LOG_NOTICE,"tick-tack\n");
return G_SOURCE_CONTINUE;
}

gpointer j_sess_watchdog(gpointer user_data){
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
