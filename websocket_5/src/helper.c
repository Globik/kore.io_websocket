//#include <kore/kore.h>
//#include <kore/http.h>
//#include <kore/tasks.h>
/*
gcc -c helper.c
# ar -cvq libdima.a  ILibWebRTC.o ILibWrapperWebRTC.o ILibAsyncSocket.o ILibParsers.o ILibAsyncUDPSocket.o ../core/utils.o

*/


#include "helper.h"

#define WEBSOCKET_PAYLOAD_SINGLE	125
#define WEBSOCKET_PAYLOAD_EXTEND_1	126
#define WEBSOCKET_PAYLOAD_EXTEND_2	127
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
