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
#include <limits.h>
#include <jansson.h>
#include "assets.h"
#define WEBSOCKET_PAYLOAD_SINGLE	125
#define WEBSOCKET_PAYLOAD_EXTEND_1	126
#define WEBSOCKET_PAYLOAD_EXTEND_2	127

int		page(struct http_request *);
int		page_ws_connect(struct http_request *);

void		websocket_connect(struct connection *);
void		websocket_disconnect(struct connection *);
void		websocket_message(struct connection *, u_int8_t, void *, size_t);
json_t *load_json(const char *, size_t);

void kore_websocket_broadcast_room(struct connection *, u_int8_t, const void *,size_t, int);
static void websocket_frame_build(struct kore_buf *, u_int8_t, const void *,size_t);

/* Websocket callbacks. */
struct kore_wscbs wscbs = {
	websocket_connect,
	websocket_message,
	websocket_disconnect
};
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

int
page(struct http_request *req)
{
	http_response_header(req, "content-type", "text/html");
	http_response(req, 200, asset_frontend_html, asset_len_frontend_html);

	return (KORE_RESULT_OK);
}

int
page_ws_connect(struct http_request *req)
{
	req->hdlr_extra=0;
	/* Perform the websocket handshake, passing our callbacks. */
	kore_log(LOG_NOTICE,"some path %s",req->path);
	kore_log(LOG_NOTICE, "%p: http_request", req);
	kore_websocket_handshake(req, &wscbs);

	return (KORE_RESULT_OK);
}
