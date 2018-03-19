//#include <stdlib.h> //exit
//#include <kore/kore.h>
#include "helper.h"
char *config_file = NULL;
char *configs_folder=NULL;
gboolean full_trickle;

//static 
	void janus_handle_signal(int signum) {
	stop_signal = signum;
	switch(g_atomic_int_get(&stop)) {
		case 0:
			JANUS_PRINT("Stopping gateway, please wait...\n");
			break;
		case 1:
			JANUS_PRINT("In a hurry? I'm trying to free resources cleanly, here!\n");
			break;
		default:
			JANUS_PRINT("Ok, leaving immediately...\n");
			break;
	}
	g_atomic_int_inc(&stop);
	if(g_atomic_int_get(&stop) > 2)
		exit(1);
}

//static
	void janus_termination_handler(void) {
	//Free the instance name, if provided 
	g_free(server_name);
	// Remove the PID file if we created it 
	janus_pidfile_remove();
	// Close the logger 
	janus_log_destroy();
	// If we're daemonizing, we send an error code to the parent 
	if(daemonize) {
		int code = 1;
		ssize_t res = 0;
		do {
			res = write(pipefd[1], &code, sizeof(int));
		} while(res == -1 && errno == EINTR);
	}
}

gint janus_is_stopping(void) {
	return g_atomic_int_get(&stop);
}

//static 
	gboolean janus_cleanup_session(gpointer user_data) {
	janus_session *session = (janus_session *) user_data;

	JANUS_LOG(LOG_INFO, "Cleaning up session %"SCNu64"...\n", session->session_id);
	janus_session_destroy(session->session_id);

	return G_SOURCE_REMOVE;
}

janus_session *janus_session_find_destroyed(guint64 session_id) {
	janus_mutex_lock(&sessions_mutex);
	janus_session *session = g_hash_table_lookup(old_sessions, &session_id);
	g_hash_table_remove(old_sessions, &session_id);
	janus_mutex_unlock(&sessions_mutex);
	return session;
}

gint janus_session_destroy(guint64 session_id) {
	janus_session *session = janus_session_find_destroyed(session_id);
	if(session == NULL) {
		JANUS_LOG(LOG_ERR, "Couldn't find session to destroy: %"SCNu64"\n", session_id);
		return -1;
	}
	JANUS_LOG(LOG_VERB, "Destroying session %"SCNu64"\n", session_id);

	/* FIXME Actually destroy session */
	janus_session_free(session);

	return 0;
}

void janus_session_free(janus_session *session) {
	if(session == NULL)
		return;
	janus_mutex_lock(&session->mutex);
	if(session->ice_handles != NULL) {
		g_hash_table_destroy(session->ice_handles);
		session->ice_handles = NULL;
	}
	if(session->source != NULL) {
		//janus_request_destroy(session->source);
		session->source = NULL;
	}
	janus_mutex_unlock(&session->mutex);
	g_free(session);
	session = NULL;
}

janus_session *janus_session_find(guint64 session_id) {
	janus_mutex_lock(&sessions_mutex);
	janus_session *session = g_hash_table_lookup(sessions, &session_id);
	janus_mutex_unlock(&sessions_mutex);
	return session;
}

janus_session *janus_session_create(guint64 session_id) {
	if(session_id == 0) {
		while(session_id == 0) {
			session_id = janus_random_uint64();
			if(janus_session_find(session_id) != NULL) {
				// Session ID already taken, try another one 
				session_id = 0;
			}
		}
	}
	JANUS_LOG(LOG_INFO, "Creating new session: %"SCNu64"\n", session_id);
	janus_session *session = (janus_session *)g_malloc0(sizeof(janus_session));
	if(session == NULL) {
		JANUS_LOG(LOG_FATAL, "Memory error!\n");
		return NULL;
	}
	session->session_id = session_id;
	session->source = NULL;
	g_atomic_int_set(&session->destroy, 0);
	g_atomic_int_set(&session->timeout, 0);
	session->last_activity = janus_get_monotonic_time();
	janus_mutex_init(&session->mutex);
	janus_mutex_lock(&sessions_mutex);
	g_hash_table_insert(sessions, janus_uint64_dup(session->session_id), session);
	janus_mutex_unlock(&sessions_mutex);
	return session;
}

//static 
	gpointer janus_sessions_watchdog(gpointer user_data) {
	GMainLoop *loop = (GMainLoop *) user_data;
	GMainContext *watchdog_context = g_main_loop_get_context(loop);
	GSource *timeout_source;

	timeout_source = g_timeout_source_new_seconds(2);
	g_source_set_callback(timeout_source, janus_check_sessions, watchdog_context, NULL);
	g_source_attach(timeout_source, watchdog_context);
	g_source_unref(timeout_source);

	JANUS_LOG(LOG_INFO, "Sessions watchdog started\n");

	g_main_loop_run(loop);

	return NULL;
}
//kore_websocket_broadcast(NULL,WEBSOCKET_OP_TEXT,buf,size,WEBSOCKET_BROADCAST_GLOBAL);
static void websocket_frame_build(struct kore_buf *, u_int8_t, const void *,size_t);
static void kore_websocket_broadcast_target(guint64,u_int8_t,const void*,size_t,int);
static void kore_websocket_broadcast_target(guint64 sender_id, u_int8_t op, const void *data,size_t len, int scope)
{
	JANUS_LOG(LOG_WARN, "[%"SCNu64"] Entering in broadcast_target\n", sender_id);
	struct connection	*c;
	struct kore_buf		*frame;
	frame = kore_buf_alloc(len);
	websocket_frame_build(frame, op, data, len);

	TAILQ_FOREACH(c, &connections, list) {
		if (c->proto == CONN_PROTO_WEBSOCKET) {
			g_print("\n ME TOO!\n");
			if(c->hdlr_extra !=NULL){
			struct ex*t=c->hdlr_extra;
				g_print("\nSOME HDLR_EXTRA STUFF. id: %d, c->proto: %d c->sender_id: %"SCNu64"\n",t->id,c->proto,t->sender_id);
			
			if(t->sender_id==sender_id) {
				g_print("\n SHould be sending only ONCE!\n");
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

//static 
	gboolean janus_check_sessions(gpointer user_data) {
	g_print("tick-tack\n");
	if(session_timeout < 1)		//Session timeouts are disabled 
		return G_SOURCE_CONTINUE;
	janus_mutex_lock(&sessions_mutex);
	if(sessions && g_hash_table_size(sessions) > 0) {
		GHashTableIter iter;
		gpointer value;
		g_hash_table_iter_init(&iter, sessions);
		while (g_hash_table_iter_next(&iter, NULL, &value)) {
			janus_session *session = (janus_session *) value;
			if (!session || g_atomic_int_get(&session->destroy)) {
				continue;
			}
			gint64 now = janus_get_monotonic_time();
			if (now - session->last_activity >= (gint64)session_timeout * G_USEC_PER_SEC &&
					!g_atomic_int_compare_and_exchange(&session->timeout, 0, 1)) {
				JANUS_LOG(LOG_INFO, "Timeout expired for session %"SCNu64"...\n", session->session_id);
				// Mark the session as over, we'll deal with it later *
				janus_session_schedule_destruction(session, FALSE, FALSE, FALSE);
				// Notify the transport 
				if(session->source) {
					json_t *event = json_object();
					json_object_set_new(event, "janus", json_string("timeout"));
					json_object_set_new(event, "session_id", json_integer(session->session_id));
					// Send this to the transport client 
					session->source->transport->send_message(session->source->instance, NULL, FALSE, event);
					// Notify the transport plugin about the session timeout 
					session->source->transport->session_over(session->source->instance, session->session_id, TRUE);
				}
				// Notify event handlers as well 
				if(janus_events_is_enabled())
					janus_events_notify_handlers(JANUS_EVENT_TYPE_SESSION, session->session_id, "timeout", NULL);

				//FIXME Is this safe? apparently it causes hash table errors on the console 
				g_hash_table_iter_remove(&iter);
				g_hash_table_replace(old_sessions, janus_uint64_dup(session->session_id), session);
			}
		}
	}
	janus_mutex_unlock(&sessions_mutex);

	return G_SOURCE_CONTINUE;
}


//static 
	void janus_session_schedule_destruction(janus_session *session,
		gboolean lock_sessions, gboolean remove_key, gboolean notify_transport) {
	if(session == NULL || !g_atomic_int_compare_and_exchange(&session->destroy, 0, 1))
		return;
	if(lock_sessions)
		janus_mutex_lock(&sessions_mutex);
	// Schedule the session for deletion 
	janus_mutex_lock(&session->mutex);
	// Remove all handles 
	if(session->ice_handles != NULL && g_hash_table_size(session->ice_handles) > 0) {
		GHashTableIter iter;
		gpointer value;
		g_hash_table_iter_init(&iter, session->ice_handles);
		while(g_hash_table_iter_next(&iter, NULL, &value)) {
			janus_ice_handle *h = value;
			if(!h || g_atomic_int_get(&stop)) {
				continue;
			}
			janus_ice_handle_destroy(session, h->handle_id);
			g_hash_table_iter_remove(&iter);
		}
	}
	janus_mutex_unlock(&session->mutex);
	if(remove_key)
		g_hash_table_remove(sessions, &session->session_id);
	g_hash_table_replace(old_sessions, janus_uint64_dup(session->session_id), session);
	GSource *timeout_source = g_timeout_source_new_seconds(3);
	g_source_set_callback(timeout_source, janus_cleanup_session, session, NULL);
	g_source_attach(timeout_source, sessions_watchdog_context);
	g_source_unref(timeout_source);
	if(lock_sessions)
		janus_mutex_unlock(&sessions_mutex);
	// Notify the source that the session has been destroyed 
	if(notify_transport && session->source && session->source->transport)
		session->source->transport->session_over(session->source->instance, session->session_id, FALSE);
}

gchar *janus_get_local_ip(void) {
	return local_ip;
}

gchar *janus_get_public_ip(void) {
	// Fallback to the local IP, if we have no public one 
	return public_ip ? public_ip : local_ip;
}
void janus_set_public_ip(const char *ip) {
	// once set do not override 
	if(ip == NULL || public_ip != NULL)
		return;
	public_ip = g_strdup(ip);
}

void janus_plugin_close(gpointer key, gpointer value, gpointer user_data) {
	janus_plugin *plugin = (janus_plugin *)value;
	if(!plugin)return;
	plugin->destroy();
}

janus_plugin *janus_plugin_find(const gchar *package) {
	if(package != NULL && plugins != NULL)	// FIXME Do we need to fix the key pointer? 
		return g_hash_table_lookup(plugins, package);
	return NULL;
}

void janus_session_notify_event(janus_session *session, json_t *event) {
// ws.send(json event); to browser
	if(session != NULL && !g_atomic_int_get(&session->destroy)/* && session->source != NULL && session->source->transport != NULL*/) {
		// Send this to the transport client 
		JANUS_LOG(LOG_HUGE, "Sending event to, session->source->transport->get_package(), session->source->instance");
	//	session->source->transport->send_message(session->source->instance, NULL, FALSE, event);
	//} 
	//else {
		guint64 mmm=0;
		json_auto_t *hsessid=json_object_get(event,"sender");//guint64 handle_id
		if(hsessid && json_is_integer(hsessid)) mmm=json_integer_value(hsessid);
		JANUS_LOG(LOG_WARN,"No transport, free the event");
		size_t size=json_dumpb(event,NULL,0,0);
	if(size==0){JANUS_LOG(LOG_WARN, "json_dumpb Size is null\n");return;}
	char*buf=alloca(size);
	size=json_dumpb(event,buf,size,0);
		//JANUS_LOG(LOG_WARN,"HERE BEFORE");
		g_print("DU KUUUUUUUUUUUUUUUUUUU\n");
fwrite((char*)buf,1,size,stdout);
	g_print("\nKU KU KU!\n");
		//JANUS_LOG(LOG_WARN,"And HERE AFTER");

	//kore_websocket_send(c, 1, buf,size);
		//guint64 session_id
		//kore_task_channel_write(t,"mama\0",5);
	//	g_print("[%"SCNu64"mmmm]\n",mmm);
JANUS_LOG(LOG_WARN, "[%"SCNu64"] PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP MMM\n", mmm);
		kore_websocket_broadcast_target(mmm,WEBSOCKET_OP_TEXT,buf,size,/*WEBSOCKET_BROADCAST_GLOBAL*/9);
		json_decref(event);
		//onError: Failed to set remote answer sdp: Called in wrong state: STATE_INPROGRESS
		//{"janus": "webrtcup", "session_id": 6042085187546594, "sender": 8053318000419074}

	}
	return;
} 


void janus_plugin_notify_event(janus_plugin *plugin, janus_plugin_session *plugin_session, json_t *event) {

	if(!plugin || !event || !json_is_object(event))
		return;
	guint64 session_id = 0, handle_id = 0;
	if(plugin_session != NULL) {
		if((plugin_session < (janus_plugin_session *)0x1000) || !janus_plugin_session_is_alive(plugin_session) || plugin_session->stopped) {
			json_decref(event);
			return;
		}
		janus_ice_handle *ice_handle = (janus_ice_handle *)plugin_session->gateway_handle;
		if(!ice_handle) {
			json_decref(event);
			return;
		}
		handle_id = ice_handle->handle_id;
		janus_session *session = (janus_session *)ice_handle->session;
		if(!session) {
			json_decref(event);
			return;
		}
		session_id = session->session_id;
	}

	if(janus_events_is_enabled()) {
		janus_events_notify_handlers(JANUS_EVENT_TYPE_PLUGIN,
			session_id, handle_id, plugin->get_package(), event);
	} else {
		json_decref(event);
	}
}

void janus_pluginso_close(gpointer key, gpointer value, gpointer user_data) {
	void *plugin = value;
	if(!plugin) return;
	// FIXME We don't dlclose plugins to be sure we can detect leaks 
	//~ dlclose(plugin);
}

void janus_plugin_end_session(janus_plugin_session *plugin_session) {
	// A plugin asked to get rid of a handle 
	if((plugin_session < (janus_plugin_session *)0x1000) || !janus_plugin_session_is_alive(plugin_session) || plugin_session->stopped)
		return;
	janus_ice_handle *ice_handle = (janus_ice_handle *)plugin_session->gateway_handle;
	if(!ice_handle)
		return;
	janus_session *session = (janus_session *)ice_handle->session;
	if(!session)
		return;
	// Destroy the handle 
	janus_mutex_lock(&session->mutex);
	janus_ice_handle_destroy(session, ice_handle->handle_id);
	g_hash_table_remove(session->ice_handles, &ice_handle->handle_id);
	janus_mutex_unlock(&session->mutex);
}

void janus_plugin_close_pc(janus_plugin_session *plugin_session) {
	// A plugin asked to get rid of a PeerConnection 
	if((plugin_session < (janus_plugin_session *)0x1000) || !janus_plugin_session_is_alive(plugin_session) || plugin_session->stopped)
		return;
	janus_ice_handle *ice_handle = (janus_ice_handle *)plugin_session->gateway_handle;
	if(!ice_handle)
		return;
	if(janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_STOP)
			|| janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ALERT))
		return;
	janus_session *session = (janus_session *)ice_handle->session;
	if(!session)
		return;

	JANUS_LOG(LOG_VERB, "[%"SCNu64"] Plugin asked to hangup PeerConnection: sending alert\n", ice_handle->handle_id);
	//Send an alert on all the DTLS connections 
	janus_ice_webrtc_hangup(ice_handle, "Close PC");
}

void janus_plugin_relay_data(janus_plugin_session *plugin_session, char *buf, int len) {
	if((plugin_session < (janus_plugin_session *)0x1000) || plugin_session->stopped || buf == NULL || len < 1)
		return;
	janus_ice_handle *handle = (janus_ice_handle *)plugin_session->gateway_handle;
	if(!handle || janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_STOP)
			|| janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ALERT))
		return;
#ifdef HAVE_SCTP
	janus_ice_relay_data(handle, buf, len);
#else
	JANUS_LOG(LOG_WARN, "Asked to relay data, but Data Channels support has not been compiled...\n");
#endif
}

void janus_plugin_relay_rtp(janus_plugin_session *plugin_session, int video, char *buf, int len) {
	if((plugin_session < (janus_plugin_session *)0x1000) || plugin_session->stopped || buf == NULL || len < 1)
		return;
	janus_ice_handle *handle = (janus_ice_handle *)plugin_session->gateway_handle;
	if(!handle || janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_STOP)
			|| janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ALERT))
		return;
	janus_ice_relay_rtp(handle, video, buf, len);
}

void janus_plugin_relay_rtcp(janus_plugin_session *plugin_session, int video, char *buf, int len) {
	if((plugin_session < (janus_plugin_session *)0x1000) || plugin_session->stopped || buf == NULL || len < 1)
		return;
	janus_ice_handle *handle = (janus_ice_handle *)plugin_session->gateway_handle;
	if(!handle || janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_STOP)
			|| janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ALERT))
		return;
	janus_ice_relay_rtcp(handle, video, buf, len);
}

// Plugin callback interface   from plugin to browser sending json event
int janus_plugin_push_event(janus_plugin_session *plugin_session,
							janus_plugin *plugin,
							const char *transaction,
							json_t *message, 
							json_t *jsep) {
	//to browser from plugin
	JANUS_LOG(LOG_WARN,"ENTERING INTO JANUS_PLUGIN_PUSH_EVENT : trans = %s",transaction);
	if(!plugin || !message){JANUS_LOG(LOG_WARN,"no message and no plugin in janus_plugin_push_event");return -1;}
	 char*foo=json_dumps(message,0);
	JANUS_LOG(LOG_WARN,"incoming message: %s",foo);
	free(foo);
	
	char*foo2=json_dumps(jsep,0);
	JANUS_LOG(LOG_WARN,"incoming message: %s",foo2);
	free(foo2);
if(!plugin_session || plugin_session < (janus_plugin_session *)0x1000 || !janus_plugin_session_is_alive(plugin_session) 
   || plugin_session->stopped){
	JANUS_LOG(LOG_WARN,"janus_plugin_session is not alive in push_event");
		return -2;
}
	janus_ice_handle *ice_handle = (janus_ice_handle *)plugin_session->gateway_handle;
	if(!ice_handle || janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_STOP))
		return JANUS_ERROR_SESSION_NOT_FOUND;
	janus_session *session = ice_handle->session;
	if(!session || g_atomic_int_get(&session->destroy))
		return JANUS_ERROR_SESSION_NOT_FOUND;
	/* Make sure this is a JSON object */
	if(!json_is_object(message)) {
		JANUS_LOG(LOG_ERR, "[%"SCNu64"] Cannot push event (JSON error: not an object)\n", ice_handle->handle_id);
		return JANUS_ERROR_INVALID_JSON_OBJECT;
	}
	// Attach JSEP if possible? */
	const char *sdp_type = json_string_value(json_object_get(jsep, "type"));
	const char *sdp = json_string_value(json_object_get(jsep, "sdp"));
	gboolean restart=json_object_get(jsep,"sdp") ? json_is_true(json_object_get(jsep,"restart")) : FALSE;
	json_t *merged_jsep = NULL;
	if(sdp_type != NULL && sdp != NULL) {
		merged_jsep = janus_plugin_handle_sdp(plugin_session, plugin, sdp_type, sdp,restart);
		if(merged_jsep == NULL) {
			if(ice_handle == NULL || janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_STOP)
					|| janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ALERT)) {
				JANUS_LOG(LOG_ERR, "[%"SCNu64"] Cannot push event (handle not available anymore or negotiation stopped)\n", ice_handle->handle_id);
				return JANUS_ERROR_HANDLE_NOT_FOUND;
			} else {
				JANUS_LOG(LOG_ERR, "[%"SCNu64"] Cannot push event (JSON error: problem with the SDP)\n", ice_handle->handle_id);
				return JANUS_ERROR_JSEP_INVALID_SDP;
			}
		}
	}
	// Reference the payload, as the plugin may still need it and will do a decref itself 
	json_incref(message);
	// Prepare JSON event 
	json_t *event = json_object();
	json_object_set_new(event, "janus", json_string("event"));
	json_object_set_new(event, "session_id", json_integer(session->session_id));
	json_object_set_new(event, "sender", json_integer(ice_handle->handle_id));
	if(transaction != NULL)json_object_set_new(event, "transaction", json_string(transaction));
	json_t *plugin_data = json_object();
	json_object_set_new(plugin_data, "plugin", json_string(plugin->get_package()));
	json_object_set_new(plugin_data, "data", message);
	json_object_set_new(event, "plugindata", plugin_data);
	if(merged_jsep != NULL) json_object_set_new(event, "jsep", merged_jsep);
	// Send the event 
	JANUS_LOG(LOG_VERB, "[%"SCNu64"] Sending event to transport...\n", ice_handle->handle_id);
	janus_session_notify_event(session, event);

	if(jsep != NULL && janus_events_is_enabled()) {
		JANUS_LOG(LOG_WARN," Notify event handlers as well."); 
		janus_events_notify_handlers(JANUS_EVENT_TYPE_JSEP,
			session->session_id, ice_handle->handle_id, "local", sdp_type, sdp);
	}

	return JANUS_OK;
}


void plug_fucker(GHashTable *pl,janus_plugin*janus_plugin){
if(pl == NULL){
pl = g_hash_table_new(g_str_hash, g_str_equal);
if(pl==NULL){g_print("FUCKER IS NULL\n");}else{
	g_print("PLUGINS IS NOT NULL\n");
	plugins=pl;
g_print("5sss %s\n",janus_plugin->get_package());}
g_hash_table_insert(pl, (gpointer)janus_plugin->get_package(), janus_plugin);
			}
}
			
gchar * select_local_ip(){
	
	if(local_ip == NULL) {
		local_ip = janus_network_detect_local_ip_as_string(janus_network_query_options_any_ip);
		
		if(local_ip == NULL) {
			g_print("Couldn't find any address! using 127.0.0.1 as the local IP... (which is NOT going to work out of your machine)\n");
			local_ip = g_strdup("127.0.0.1");
			//local_ip=loc_ip;
			return local_ip;
		}else{return local_ip;}
	}
}

void conf_max_nack_queue(){
g_print("entering conf_max\n");
item = janus_config_get_item_drilldown(config, "media", "max_nack_queue");
if(item && item->value) {
g_print("There is item and item value!\n");
int mnq = atoi(item->value);
if(mnq < 0) {
JANUS_LOG(LOG_WARN, "Ignoring max_nack_queue value as it's not a positive integer\n");
} else if(mnq > 0 && mnq < 200) {
JANUS_LOG(LOG_WARN, "Ignoring max_nack_queue value as it's less than 200\n");
} else {
janus_set_max_nack_queue(mnq);
}
}else{g_print("There is not item and item value!\n");}
g_print("leaving out conf_max\n");
}

void conf_force_bundle_or_and_rtcp_mux(){
	/*
item = janus_config_get_item_drilldown(config, "media", "force-bundle");
	force_bundle = (item && item->value) ? janus_is_true(item->value) : FALSE;
	janus_ice_force_bundle(force_bundle);
	item = janus_config_get_item_drilldown(config, "media", "force-rtcp-mux");
	force_rtcpmux = (item && item->value) ? janus_is_true(item->value) : FALSE;
	janus_ice_force_rtcpmux(force_rtcpmux);
	*/
}
void conf_no_media_timer(){
	JANUS_LOG(LOG_WARN, "entering no_media_timer\n");
item = janus_config_get_item_drilldown(config, "media", "no_media_timer");
	if(item && item->value) {
		int nmt = atoi(item->value);
		if(nmt < 0) {
			JANUS_LOG(LOG_WARN, "Ignoring no_media_timer value as it's not a positive integer\n");
		} else {
			janus_set_no_media_timer(nmt);
		}
	}

}

const char*conf_cert_pem(){
const char*cert_pem;
item = janus_config_get_item_drilldown(config, "certificates", "cert_pem");
	if(!item || !item->value) {
		g_print("\nserver_pem = NULL;\n");
		cert_pem=NULL;
		return cert_pem;
	} else {
		g_print("server_pem = item->value;\n");
		cert_pem=item->value;
		g_print("cert_pem %s\n",cert_pem);
		return cert_pem;
	}
}

const char*conf_cert_key(){
	const char* cert_key;
item = janus_config_get_item_drilldown(config, "certificates", "cert_key");
	if(!item || !item->value) {
		//server_key = NULL;
		cert_key=NULL;
		return cert_key;
	} else {
		g_print("server_key = item->value;\n");
		cert_key=item->value;
		g_print("cert_key %s\n",cert_key);
		return cert_key;
	}
}

void conf_dtls_mtu(){
item = janus_config_get_item_drilldown(config, "media", "dtls_mtu");
if(item && item->value)janus_dtls_bio_filter_set_mtu(atoi(item->value));
}


void test_private_address(){
	if(stun_server == NULL && turn_server == NULL) {
		//No STUN and TURN server provided for Janus: make sure it isn't on a private address 
		gboolean private_address = FALSE;
		const char *test_ip = nat_1_1_mapping ? nat_1_1_mapping : local_ip;
		janus_network_address addr;
		if(janus_network_string_to_address(janus_network_query_options_any_ip, test_ip, &addr) != 0) {
			JANUS_LOG(LOG_ERR, "Invalid address %s..?\n", test_ip);
		} else {
			if(addr.family == AF_INET) {
				unsigned short int ip[4];
				sscanf(test_ip, "%hu.%hu.%hu.%hu", &ip[0], &ip[1], &ip[2], &ip[3]);
				if(ip[0] == 10) {
					// Class A private address 
					private_address = TRUE;
				} else if(ip[0] == 172 && (ip[1] >= 16 && ip[1] <= 31)) {
					// Class B private address 
					private_address = TRUE;
				} else if(ip[0] == 192 && ip[1] == 168) {
					// Class C private address 
					private_address = TRUE;
				}
			} else {
				// TODO Similar check for IPv6... 
			}
		}
		if(private_address) {
			JANUS_LOG(LOG_WARN, "Janus is deployed on a private address (%s) but you didn't specify any STUN server!"
			                  		                    " Expect trouble if this is supposed to work over the internet and not just in a LAN...\n", test_ip);
		}
	}
	
	
}

void conf_nice_debug(){
item = janus_config_get_item_drilldown(config, "nat", "nice_debug");
if(item && item->value && janus_is_true(item->value)) {
g_print("Enable libnice debugging\n");
janus_ice_debugging_enable();
	}
}


void conf_turn(){
//item=janus_config_get_item_drilldown(config,"media","ipv6");
item = janus_config_get_item_drilldown(config, "media", "rtp_port_range");
if(item && item->value) {
// Split in min and max port 
		char *maxport = strrchr(item->value, '-');
		if(maxport != NULL) {
			*maxport = '\0';
			maxport++;
			rtp_min_port = atoi(item->value);
			rtp_max_port = atoi(maxport);
			maxport--;
			*maxport = '-';
		}
		if(rtp_min_port > rtp_max_port) {
			uint16_t temp_port = rtp_min_port;
			rtp_min_port = rtp_max_port;
			rtp_max_port = temp_port;
		}
		if(rtp_max_port == 0)
			rtp_max_port = 65535;
		JANUS_LOG(LOG_INFO, "RTP port range: %u -- %u\n", rtp_min_port, rtp_max_port);
	}
	
	
	
	JANUS_LOG(LOG_WARN," Check if we need to enable the ICE Lite mode \n");
	item = janus_config_get_item_drilldown(config, "nat", "ice_lite");
	ice_lite =(item && item->value) ? janus_is_true(item->value) : FALSE;
	//Check if we need to enable ICE-TCP support (warning: still broken, for debugging only) 
item = janus_config_get_item_drilldown(config, "nat", "ice_tcp");
	ice_tcp = (item && item->value) ? janus_is_true(item->value) : FALSE;
	// Any STUN server to use in Janus? 
	item = janus_config_get_item_drilldown(config, "nat", "stun_server");
	if(item && item->value)
		stun_server = (char *)item->value;
	item = janus_config_get_item_drilldown(config, "nat", "stun_port");
	if(item && item->value)
		stun_port = atoi(item->value);
	//Any 1:1 NAT mapping to take into account? 
	item = janus_config_get_item_drilldown(config, "nat", "nat_1_1_mapping");
	if(item && item->value) {
		g_print("Using nat_1_1_mapping for public ip - %s\n", item->value);
		if(!janus_network_string_is_valid_address(janus_network_query_options_any_ip, item->value)) {
			g_print("Invalid nat_1_1_mapping address %s, disabling...\n", item->value);
		} else {
			nat_1_1_mapping = item->value;
			janus_set_public_ip(item->value);
			janus_ice_enable_nat_1_1();
		}
	}
	// Any TURN server to use in Janus? 
	item = janus_config_get_item_drilldown(config, "nat", "turn_server");
	if(item && item->value)
		turn_server = (char *)item->value;
	item = janus_config_get_item_drilldown(config, "nat", "turn_port");
	if(item && item->value)
		turn_port = atoi(item->value);
	item = janus_config_get_item_drilldown(config, "nat", "turn_type");
	if(item && item->value)
		turn_type = (char *)item->value;
	item = janus_config_get_item_drilldown(config, "nat", "turn_user");
	if(item && item->value)
		turn_user = (char *)item->value;
	item = janus_config_get_item_drilldown(config, "nat", "turn_pwd");
	if(item && item->value)
		turn_pwd = (char *)item->value;
	// Check if there's any TURN REST API backend to use 
	item = janus_config_get_item_drilldown(config, "nat", "turn_rest_api");
	if(item && item->value)
		turn_rest_api = (char *)item->value;
	item = janus_config_get_item_drilldown(config, "nat", "turn_rest_api_key");
	if(item && item->value)
		turn_rest_api_key = (char *)item->value;
#ifdef HAVE_LIBCURL
	item = janus_config_get_item_drilldown(config, "nat", "turn_rest_api_method");
	if(item && item->value)
		turn_rest_api_method = (char *)item->value;
#endif

	
	// Initialize the ICE stack now 
	
	/*
	const char *nat_1_1_mapping = NULL;
	uint16_t rtp_min_port = 0, rtp_max_port = 0;
	gboolean ice_lite = FALSE, ice_tcp = FALSE, ipv6 = FALSE;
	*/
	g_print("nat 1 1 mapping %s\n",nat_1_1_mapping);
	g_print("ice_tcp: %d\n",ice_tcp);
	g_print("ice_lite %d\n",ice_lite);
	g_print("ipv6 %d\n",ipv6);
	janus_ice_init(ice_lite, ice_tcp,full_trickle, ipv6, rtp_min_port, rtp_max_port);
	if(janus_ice_set_stun_server(stun_server, stun_port) < 0) {
	JANUS_LOG(LOG_FATAL,"Invalid STUN address %s:%u\n", stun_server, stun_port);
		exit(1);
	}
	if(janus_ice_set_turn_server(turn_server, turn_port, turn_type, turn_user, turn_pwd) < 0) {
		JANUS_LOG(LOG_FATAL,"Invalid TURN address %s:%u\n", turn_server, turn_port);
		exit(1);
	}
#ifndef HAVE_LIBCURL
	if(turn_rest_api != NULL || turn_rest_api_key != NULL) {
		g_print("A TURN REST API backend specified in the settings, but libcurl support has not been built\n");
	}
#else
	if(janus_ice_set_turn_rest_api(turn_rest_api, turn_rest_api_key, turn_rest_api_method) < 0) {
		g_print("Invalid TURN REST API configuration: %s (%s, %s)\n", turn_rest_api, turn_rest_api_key, turn_rest_api_method);
		exit(1);
	}
#endif
	
	
}
	
void conf_session_timeout(){
item = janus_config_get_item_drilldown(config, "general", "session_timeout");
if(item && item->value) {
int st = atoi(item->value);
if(st < 0) {
JANUS_LOG(LOG_WARN, "Ignoring session_timeout value as it's not a positive integer\n");
} else {
if(st == 0) {
JANUS_LOG(LOG_WARN, "Session timeouts have been disabled (note, may result in orphaned sessions)\n");
}
session_timeout = st;
}}}


void conf_interface(){
item = janus_config_get_item_drilldown(config, "general", "interface");
	if(item && item->value) {
		JANUS_LOG(LOG_VERB, "  -- Will try to use %s\n", item->value);
		//Verify that the address is valid 
		struct ifaddrs *ifas = NULL;
		janus_network_address iface;
		janus_network_address_string_buffer ibuf;
		if(getifaddrs(&ifas) || ifas == NULL) {
			JANUS_LOG(LOG_ERR, "Unable to acquire list of network devices/interfaces; some configurations may not work as expected...\n");
		} else {
			if(janus_network_lookup_interface(ifas, item->value, &iface) != 0) {
				JANUS_LOG(LOG_WARN, "Error setting local IP address to %s, falling back to detecting IP address...\n", item->value);
			} else {
				if(janus_network_address_to_string_buffer(&iface, &ibuf) != 0 || janus_network_address_string_buffer_is_null(&ibuf)) {
					JANUS_LOG(LOG_WARN, "Error getting local IP address from %s, falling back to detecting IP address...\n", item->value);
				} else {
					local_ip = g_strdup(janus_network_address_string_from_buffer(&ibuf));
				}
			}
		}
	}
	
}
	

void conf_ice_ignore_list(){
item = janus_config_get_item_drilldown(config, "nat", "ice_ignore_list");
	if(item && item->value) {
		gchar **list = g_strsplit(item->value, ",", -1);
		gchar *index = list[0];
		if(index != NULL) {
			int i=0;
			while(index != NULL) {
				if(strlen(index) > 0) {
					JANUS_LOG(LOG_INFO, "Adding '%s' to the ICE ignore list...\n", index);
					janus_ice_ignore_interface(g_strdup(index));
				}
				i++;
				index = list[i];
			}
		}
		g_clear_pointer(&list, g_strfreev);
	}
}


void conf_ice_enforce_list(){
janus_config_item *item = janus_config_get_item_drilldown(config, "nat", "ice_enforce_list");
	if(item && item->value) {
		gchar **list = g_strsplit(item->value, ",", -1);
		gchar *index = list[0];
		if(index != NULL) {
			int i=0;
			while(index != NULL) {
				if(strlen(index) > 0) {
					JANUS_LOG(LOG_INFO, "Adding '%s' to the ICE enforce list...\n", index);
					janus_ice_enforce_interface(g_strdup(index));
				}
				i++;
				index = list[i];
			}
		}
		g_clear_pointer(&list, g_strfreev);
	}
	
}

void stop_signal_hook(){
signal(SIGINT, janus_handle_signal);
	signal(SIGTERM, janus_handle_signal);
	atexit(janus_termination_handler);
}

void puzomerka(gboolean using_stdout){
g_print("Janus commit: %s\n", janus_build_git_sha);
	g_print("Compiled on:  %s\n\n", janus_build_git_time);

	g_print("after use_std_out\n");
	if(janus_log_init(0,using_stdout,"logfile")<0){g_print("no logfile error\n");exit(1);}
	JANUS_PRINT("---------------------------------------------------\n");
	g_print("  Starting Meetecho Janus (WebRTC Gateway) v%s\n", JANUS_VERSION_STRING);
	JANUS_PRINT("---------------------------------------------------\n\n");
}

void set_conf_file(char *cnfolder){
	g_print("CONF FOLLLLDERRRRR!!!!!!!!!!! %s\n",cnfolder);
	config_file=NULL;
if(config_file == NULL) {
	g_print("COOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOONFIG FILE IS NULLL!!!\n");
		char file[255];
		g_snprintf(file, 255, "%s/janus.cfg", cnfolder);
		config_file = g_strdup(file);
	}else{g_print("COOOOOOOOOOOOOOOOOOOOOONFIG FIIIIIIIIIIILEEEEEEEE is NOT NULL %s\n",config_file);}
	char*bliad= "/home/globik/kore.io_websocket/websocket_10/configs";
	g_print("COOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOONF: %s\n",config_file);
	if((config = janus_config_parse(config_file)) == NULL) {
		g_print("Error reading/parsing the configuration file, going on with the defaults and the command line arguments\n");
		config = janus_config_create("janus.cfg");
		if(config == NULL) {
			g_print("If we can't even create an empty configuration, something's definitely wrong \n");
			exit(1);
		}
	}else{g_print("CONFIG FOLDER!!!!!!!!!!!!!!\n");}
}


void janus_eventhandler_close(gpointer key, gpointer value, gpointer user_data) {
	janus_eventhandler *eventhandler = (janus_eventhandler *)value;
	if(!eventhandler)
		return;
	eventhandler->destroy();
}

void janus_eventhandlerso_close(gpointer key, gpointer value, gpointer user_data) {
	void *eventhandler = (janus_eventhandler *)value;
	if(!eventhandler)
		return;
	//~ dlclose(eventhandler);
}


void fuck_up(struct kore_task*taski)
	{
	janus_log_level=7;
	gboolean use_stdout = TRUE;
	// Core dumps may be disallowed by parent of this process; change that 
	//struct rlimit core_limits;
	//core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
	//setrlimit(RLIMIT_CORE, &core_limits);
puzomerka(use_stdout);
stop_signal_hook();
configs_folder = g_strdup (CONFDIR);
g_print("configs_folder %s\n",configs_folder);
set_conf_file(configs_folder);
conf_ice_enforce_list();
	conf_ice_ignore_list();
conf_interface();
	#if !GLIB_CHECK_VERSION(2, 36, 0)
	g_type_init();
#endif
	janus_log_timestamps = TRUE;
	janus_log_colors = TRUE;
// What is the local IP? 
g_print("Selecting local IP address...\n");
	if(local_ip==NULL)local_ip=select_local_ip();
conf_session_timeout();
g_print("Using %s as local IP...\n", local_ip);
// Is there any API secret to consider? 
	api_secret = NULL;
// Setup ICE stuff (e.g., checking if the provided STUN server is correct) 
	char *stun_server = NULL, *turn_server = NULL;
	uint16_t stun_port = 0, turn_port = 0;
	char *turn_type = NULL, *turn_user = NULL, *turn_pwd = NULL;
	char *turn_rest_api = NULL, *turn_rest_api_key = NULL;
#ifdef HAVE_LIBCURL
	char *turn_rest_api_method = NULL;
#endif
const char *nat_1_1_mapping = NULL;
uint16_t rtp_min_port = 0, rtp_max_port = 0;
gboolean ice_lite = FALSE,full_trickle=FALSE, ice_tcp = FALSE, ipv6 = FALSE;
item = janus_config_get_item_drilldown(config, "media", "ipv6");
ipv6 = (item && item->value) ? janus_is_true(item->value) : FALSE;
conf_turn();
conf_nice_debug();
test_private_address();
// Are we going to force BUNDLE and/or rtcp-mux? nn
gboolean force_bundle = FALSE, force_rtcpmux = FALSE;
conf_force_bundle_or_and_rtcp_mux();
// NACK related stuff 
conf_max_nack_queue();
//no-media timer 
conf_no_media_timer();
		/* RFC4588 support */
	item = janus_config_get_item_drilldown(config, "media", "rfc_4588");
	if(item && item->value) {
		janus_set_rfc4588_enabled(janus_is_true(item->value));
	}
g_print("Setup OpenSSL stuff\n");
const char* server_pem=conf_cert_pem();
		//g_print
const char* server_key=conf_cert_key();
g_print("Using certificates:\n\t%s\n\t%s\n", server_pem, server_key);
SSL_library_init();
SSL_load_error_strings();
OpenSSL_add_all_algorithms();
// ... and DTLS-SRTP in particular 
// janus_use_openssl_pre_1_1_api	
		
//guint a=0;		
		const char*password=NULL;
guint a=janus_dtls_srtp_init(server_pem, server_key,password);
g_print("aaa %d\n",a);
if(a < 0) {
g_print("SRTP_INIT IS NOT OK!!!!! EXIT!\n");
exit(1);
}else{
g_print("SRTP INIT OK!\n");
}


	g_print("Check if there's any custom value for the starting MTU to use in the BIO filter\n"); 
conf_dtls_mtu();

#ifdef HAVE_SCTP
	// Initialize SCTP for DataChannels 
	if(janus_sctp_init() < 0) {exit(1);}else{g_print("janus_sctp_init - OK\n");}
#else
	JANUS_LOG(LOG_WARN, "Data Channels support not compiled????????????????????????????????????\n");
#endif
// Sessions 
	sessions = g_hash_table_new_full(g_int64_hash, g_int64_equal, (GDestroyNotify)g_free, NULL);
	old_sessions = g_hash_table_new_full(g_int64_hash, g_int64_equal, (GDestroyNotify)g_free, NULL);
	janus_mutex_init(&sessions_mutex);
	// Start the sessions watchdog 
	sessions_watchdog_context = g_main_context_new();
	GMainLoop *watchdog_loop = g_main_loop_new(sessions_watchdog_context, FALSE);
	GError *error = NULL;
	GThread *watchdog = g_thread_try_new("sessions watchdog", &janus_sessions_watchdog, watchdog_loop, &error);
	if(error != NULL) {
		JANUS_LOG(LOG_FATAL, "Got error %d (%s) trying to start sessions watchdog...\n", error->code, error->message ? error->message : "??");
		exit(1);
	}

gchar **disabled_plugins = NULL;
//struct dirent *
	pluginent = NULL;
disabled_plugins = NULL;
const char *path=NULL;
//DIR *
	dir=NULL;
// Load plugins 
//	path = PLUGINDIR;
//path="/usr/local/lib/janus/plugins";
//path="/home/globik/webrtc/plugins";
path="/home/globik/kore.io_websocket/websocket_10/plugins";
load_plugin(path);
if(disabled_plugins != NULL)g_strfreev(disabled_plugins);
disabled_plugins = NULL;
gboolean janus_api_enabled = FALSE, admin_api_enabled = FALSE;
	
while(!g_atomic_int_get(&stop)) {
		usleep(250000); 
		// A signal will cancel usleep() but not g_usleep() 
	}
// Done 
	JANUS_LOG(LOG_INFO, "Ending watchdog mainloop...\n");
	g_main_loop_quit(watchdog_loop);
	g_thread_join(watchdog);
	watchdog = NULL;
	g_main_loop_unref(watchdog_loop);
	g_main_context_unref(sessions_watchdog_context);
	sessions_watchdog_context = NULL;

	if(config)
		janus_config_destroy(config);

		/*
	JANUS_LOG(LOG_INFO, "Closing transport plugins:\n");
	if(transports != NULL) {
		g_hash_table_foreach(transports, janus_transport_close, NULL);
		g_hash_table_destroy(transports);
	}
	if(transports_so != NULL) {
		g_hash_table_foreach(transports_so, janus_transportso_close, NULL);
		g_hash_table_destroy(transports_so);
	}
*/
	JANUS_LOG(LOG_INFO, "Destroying sessions...\n");
	g_clear_pointer(&sessions, g_hash_table_destroy);
	g_clear_pointer(&old_sessions, g_hash_table_destroy);
	JANUS_LOG(LOG_INFO, "Freeing crypto resources...\n");
	janus_dtls_srtp_cleanup();
	EVP_cleanup();
	ERR_free_strings();
#ifdef HAVE_SCTP
	JANUS_LOG(LOG_INFO, "De-initializing SCTP...\n");
	janus_sctp_deinit();
#endif
	janus_ice_deinit();
	//janus_auth_deinit();

	JANUS_LOG(LOG_INFO, "Closing plugins.\n");
	if(plugins != NULL) {
		JANUS_LOG(LOG_WARN, "PLUGINS NOT NULL, DELETENG NOW!!!!!!!!!!!!!!!!\n");
		g_hash_table_foreach(plugins, janus_plugin_close, NULL);
		g_hash_table_destroy(plugins);
	}else{JANUS_LOG(LOG_INFO, "Closing plugins: PLUGINS IS NULL\n");}
	if(plugins_so != NULL) {
		JANUS_LOG(LOG_INFO, "PLUGINS_SO:\n");
		g_hash_table_foreach(plugins_so, janus_pluginso_close, NULL);
		g_hash_table_destroy(plugins_so);
	}else{JANUS_LOG(LOG_INFO, "Closing plugins: PLUGINS_SO IS NULL\n");}
	JANUS_LOG(LOG_INFO, "Closing event handlers:\n");
	janus_events_deinit();
	if(eventhandlers != NULL) {
		g_hash_table_foreach(eventhandlers, janus_eventhandler_close, NULL);
		g_hash_table_destroy(eventhandlers);
	}
	if(eventhandlers_so != NULL) {
		g_hash_table_foreach(eventhandlers_so, janus_eventhandlerso_close, NULL);
		g_hash_table_destroy(eventhandlers_so);
	}
g_free(local_ip);
JANUS_PRINT("Bye!\n");
exit(0);

}
