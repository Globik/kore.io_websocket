#include "helper.h"
//ss#include <kore/kore.h>
//type "message"
void janus_process_e(struct connection*,json_t*,char*);
void janus_process_e(struct connection*ws_cl,json_t*root, char*grund){
json_auto_t *reply=json_object();
json_object_set_new(reply, "type", json_string("janus_error"));
json_object_set_new(reply,"janus_err_msg",json_string(grund));
size_t size=json_dumpb(reply,NULL,0,0);
if(size==0){JANUS_LOG(LOG_WARN, "json_dumpb Size is null\n");json_decref(root);return;}
char*buf=alloca(size);
size=json_dumpb(reply,buf,size,0);
kore_websocket_send(ws_cl, 1, buf,size);
json_decref(root);
return;
}

void incoming_message(janus_ice_handle*handle,janus_session*session,json_t*root,guint64 session_id,struct connection*ws_cl) {
if(handle == NULL) {
janus_process_e(ws_cl,root,"Unhandled request at this path. Hndle is null.");
goto jsondone;
}
if(handle->app == NULL || handle->app_handle == NULL) {
janus_process_e(ws_cl,root,"No plugin to handle this message");
goto jsondone;
}
janus_plugin *plugin_t = (janus_plugin *)handle->app;
JANUS_LOG(LOG_VERB, "[%"SCNu64"] There's a message for %s\n", handle->handle_id, plugin_t->get_name());
json_t *body = json_object_get(root, "body");
// Is there an SDP attached? 
json_t *jsep = json_object_get(root, "jsep");
char *jsep_type = NULL;
char *jsep_sdp = NULL, *jsep_sdp_stripped = NULL;
gboolean renegotiation = FALSE;
if(jsep != NULL) {
if(!json_is_object(jsep)) {
janus_process_e(ws_cl, root,"Invalid jsep object");
goto jsondone;
}

json_t *type = json_object_get(jsep, "type");
jsep_type = g_strdup(json_string_value(type));
type = NULL;
gboolean do_trickle = TRUE;
json_t *jsep_trickle = json_object_get(jsep, "trickle");
do_trickle = jsep_trickle ? json_is_true(jsep_trickle) : TRUE;
// Are we still cleaning up from a previous media session? 
if(janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_CLEANING)) {
JANUS_LOG(LOG_VERB, "[%"SCNu64"] Still cleaning up from a previous media session, let's wait a bit...\n", handle->handle_id);
gint64 waited = 0;
while(janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_CLEANING)) {
g_usleep(100000);
waited += 100000;
if(waited >= 3*G_USEC_PER_SEC) {
JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- Waited 3 seconds, that's enough!\n", handle->handle_id);
janus_process_e(ws_cl,root,"Still cleaning a previous session");
goto jsondone;
}
}
}
//Check the JSEP type 
janus_mutex_lock(&handle->mutex);
int offer = 0;
if(!strcasecmp(jsep_type, "offer")) {
				offer = 1;
				janus_flags_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
				janus_flags_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_OFFER);
				janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_ANSWER);
			} 
else if(!strcasecmp(jsep_type, "answer")) {
janus_flags_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_ANSWER);
offer = 0;
			} 
else {
// TODO Handle other message types as well 
janus_process_e(ws_cl,root,"JSEP error: unknown message type,not offer,nor answer.");
g_free(jsep_type);
janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
janus_mutex_unlock(&handle->mutex);
goto jsondone;
}
json_t *sdp = json_object_get(jsep, "sdp");
jsep_sdp = (char *)json_string_value(sdp);
JANUS_LOG(LOG_VERB, "[%"SCNu64"] Remote SDP:\n%s", handle->handle_id, jsep_sdp);
char error_str[512];
int audio = 0, video = 0, data = 0;
janus_sdp *parsed_sdp = janus_sdp_preparse(jsep_sdp, error_str, sizeof(error_str), &audio, &video, &data);
if(parsed_sdp == NULL) {
janus_process_e(ws_cl, root, error_str);
g_free(jsep_type);
janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
janus_mutex_unlock(&handle->mutex);
goto jsondone;
}

//if(janus_events_is_enabled()) {
//janus_events_notify_handlers(JANUS_EVENT_TYPE_JSEP,
//session_id, handle_id, handle->opaque_id, "remote", jsep_type, jsep_sdp);}

// FIXME We're only handling single audio/video lines for now... 
JANUS_LOG(LOG_VERB, "[%"SCNu64"] Audio %s been negotiated, Video %s been negotiated, SCTP/DataChannels %s been negotiated\n",
handle->handle_id,audio ? "has" : "has NOT",video ? "has" : "has NOT",data ? "have" : "have NOT");
if(audio > 1) {
				JANUS_LOG(LOG_WARN, "[%"SCNu64"] More than one audio line? only going to negotiate one...\n", handle->handle_id);
}
if(video > 1) {
				JANUS_LOG(LOG_WARN, "[%"SCNu64"] More than one video line? only going to negotiate one...\n", handle->handle_id);
			}
if(data > 1) {
				JANUS_LOG(LOG_WARN, "[%"SCNu64"] More than one data line? only going to negotiate one...\n", handle->handle_id);
			}
#ifndef HAVE_SCTP
if(data) {
JANUS_LOG(LOG_WARN, "[%"SCNu64"]   -- DataChannels have been negotiated, but support for them has not been compiled...\n", handle->handle_id);
			}
#endif
//Check if it's a new session, or an update...
if(!janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_READY)|| janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ALERT)) {
//New session 
if(offer) {
//Setup ICE locally (we received an offer) 
if(janus_ice_setup_local(handle, offer, audio, video, data, do_trickle) < 0) {
JANUS_LOG(LOG_ERR, "Error setting ICE locally\n");
janus_sdp_free(parsed_sdp);
g_free(jsep_type);
janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
janus_process_e(ws_cl,root,"Error setting ICE locally");
janus_mutex_unlock(&handle->mutex);
goto jsondone;
}
}
else {
//Make sure we're waiting for an ANSWER in the first place 
if(!handle->agent) {
JANUS_LOG(LOG_ERR, "Unexpected ANSWER (did we offer?)\n");
janus_sdp_free(parsed_sdp);
g_free(jsep_type);
janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
janus_process_e(ws_cl,root,"Unexpected ANSWER (did we offer?)");
janus_mutex_unlock(&handle->mutex);
goto jsondone;
}
}
if(janus_sdp_process(handle, parsed_sdp, FALSE) < 0) {
JANUS_LOG(LOG_ERR, "Error processing SDP\n");
janus_sdp_free(parsed_sdp);
g_free(jsep_type);
janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
janus_process_e(ws_cl, root,"Error processing SDP");
janus_mutex_unlock(&handle->mutex);
goto jsondone;
}
if(!offer) {
//Set remote candidates now (we received an answer)
janus_flags_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_TRICKLE);
//We got our answer
janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
//Any pending trickles?
if(handle->pending_trickles) {
JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- Processing %d pending trickle candidates\n", handle->handle_id, g_list_length(handle->pending_trickles));
GList *temp = NULL;
while(handle->pending_trickles) {
temp = g_list_first(handle->pending_trickles);
handle->pending_trickles = g_list_remove_link(handle->pending_trickles, temp);
							janus_ice_trickle *trickle = (janus_ice_trickle *)temp->data;
							g_list_free(temp);
							if(trickle == NULL)
								continue;
							if((janus_get_monotonic_time() - trickle->received) > 45*G_USEC_PER_SEC) {
								janus_ice_trickle_destroy(trickle);
								continue;
							}
							json_t *candidate = trickle->candidate;
							if(candidate == NULL) {
								janus_ice_trickle_destroy(trickle);
								continue;
							}
							if(json_is_object(candidate)) {
		//We got a single candidate 
								int error = 0;
								const char *error_string = NULL;
								if((error = janus_ice_trickle_parse(handle, candidate, &error_string)) != 0) {
//FIXME We should report the error parsing the trickle candidate */
								}
							} else if(json_is_array(candidate)) {
							// We got multiple candidates in an array 
								JANUS_LOG(LOG_VERB, "Got multiple candidates (%zu)\n", json_array_size(candidate));
								if(json_array_size(candidate) > 0) {
			
									size_t i = 0;
									for(i=0; i<json_array_size(candidate); i++) {
										json_t *c = json_array_get(candidate, i);
//FIXME We don't care if any trickle fails to parse 
										janus_ice_trickle_parse(handle, c, NULL);
									}
								}
							}
							/* Done, free candidate */
							janus_ice_trickle_destroy(trickle);
						}
					}
//This was an answer, check if it's time to start ICE
if(janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_TRICKLE) &&
							!janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ALL_TRICKLES)) {
						JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- ICE Trickling is supported by the browser, waiting for remote candidates...\n", handle->handle_id);
						janus_flags_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_START);
} else {
	JANUS_LOG(LOG_VERB, "[%"SCNu64"] Done! Sending connectivity checks...\n", handle->handle_id);
if(handle->stream_id > 0) {
janus_ice_setup_remote_candidates(handle, handle->stream_id, 1);
						}
}
} else {
					//Check if transport wide CC is supported
					int transport_wide_cc_ext_id = janus_rtp_header_extension_get_id(jsep_sdp, JANUS_RTP_EXTMAP_TRANSPORT_WIDE_CC);
					handle->stream->do_transport_wide_cc = TRUE;
					handle->stream->transport_wide_cc_ext_id = transport_wide_cc_ext_id;
				}
} else {
//FIXME This is a renegotiation: we can currently only handle simple changes in media
//direction and ICE restarts: anything more complex than that will result in an error 
JANUS_LOG(LOG_INFO, "[%"SCNu64"] Negotiation update, checking what changed...\n", handle->handle_id);
if(janus_sdp_process(handle, parsed_sdp, TRUE) < 0) {
JANUS_LOG(LOG_ERR, "Error processing SDP\n");
janus_sdp_free(parsed_sdp);
g_free(jsep_type);
janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
janus_process_e(ws_cl, root,"Error processing SDP");
janus_mutex_unlock(&handle->mutex);
goto jsondone;
}
renegotiation = TRUE;
if(janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ICE_RESTART)) {
JANUS_LOG(LOG_INFO, "[%"SCNu64"] Restarting ICE...\n", handle->handle_id);
//Update remote credentials for ICE
if(handle->stream) {
nice_agent_set_remote_credentials(handle->agent, handle->stream->stream_id,handle->stream->ruser, handle->stream->rpass);
}
//FIXME We only need to do that for offers: if it's an answer, we did that already */
if(offer) {
janus_ice_restart(handle);
} else {
janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ICE_RESTART);
}
//If we're full-trickling, we'll need to resend the candidates later
if(janus_ice_is_full_trickle_enabled()) {
janus_flags_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_RESEND_TRICKLES);
}
}
#ifdef HAVE_SCTP
if(!offer) {
// Were datachannels just added?
if(janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_DATA_CHANNELS)) {
janus_ice_stream *stream = handle->stream;
if(stream != NULL && stream->component != NULL && stream->component->dtls != NULL && stream->component->dtls->sctp == NULL) {
JANUS_LOG(LOG_WARN, "[%"SCNu64"] Creating datachannels...\n", handle->handle_id);
janus_dtls_srtp_create_sctp(stream->component->dtls);
}
}
}
#endif
}
char *tmp = handle->remote_sdp;
handle->remote_sdp = g_strdup(jsep_sdp);
g_free(tmp);
janus_mutex_unlock(&handle->mutex);
if(janus_sdp_anonymize(parsed_sdp) < 0) {
janus_process_e(ws_cl, root,"JSEP error: invalid SDP");
janus_sdp_free(parsed_sdp);
g_free(jsep_type);
janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
goto jsondone;
}
jsep_sdp_stripped = janus_sdp_write(parsed_sdp);
janus_sdp_free(parsed_sdp);
sdp = NULL;
janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
}
// here figure skobka?
//Make sure the app handle is still valid
if(handle->app == NULL || handle->app_handle == NULL || !janus_plugin_session_is_alive(handle->app_handle)) {
janus_process_e(ws_cl, root,"No plugin to handle this message");
g_free(jsep_type);
g_free(jsep_sdp_stripped);
janus_flags_clear(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
goto jsondone;
}

//Send the message to the plugin (which must eventually free transaction_text and unref the two objects, body and jsep) 
json_incref(body);
json_t *body_jsep = NULL;
if(jsep_sdp_stripped) {
body_jsep = json_pack("{ssss}", "type", jsep_type, "sdp", jsep_sdp_stripped);
// Check if VP8 simulcasting is enabled 
if(janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_HAS_VIDEO)) {
if(handle->stream && handle->stream->video_ssrc_peer[1]) {
json_t *simulcast = json_object();
json_object_set(simulcast, "ssrc-0", json_integer(handle->stream->video_ssrc_peer[0]));
json_object_set(simulcast, "ssrc-1", json_integer(handle->stream->video_ssrc_peer[1]));
if(handle->stream->video_ssrc_peer[2]) json_object_set(simulcast, "ssrc-2", json_integer(handle->stream->video_ssrc_peer[2]));
json_object_set(body_jsep, "simulcast", simulcast);
}
}
// Check if this is a renegotiation or update 
if(renegotiation)
json_object_set(body_jsep,"update", json_true());
}
janus_plugin_result *result = plugin_t->handle_message(handle->app_handle,g_strdup((char *)"trans_janus"), body, body_jsep);
g_free(jsep_type);
g_free(jsep_sdp_stripped);
if(result == NULL) {
// Something went horribly wrong! 
janus_process_e(ws_cl,root,"Plugin didn't give a result is NULL.");
goto jsondone;
}
if(result->type == JANUS_PLUGIN_OK) {
//The plugin gave a result already (synchronous request/response) 
if(result->content == NULL || !json_is_object(result->content)) {
//Missing content, or not a JSON object
janus_process_e(ws_cl, root,"Plugin didn't provide any content for this synchronous response Plugin returned an invalid JSON response");
janus_plugin_result_destroy(result);
goto jsondone;
}
// Reference the content, as destroying the result instance will decref it
			json_incref(result->content);
			// Prepare JSON response 
			json_t *reply = json_object();
			json_object_set_new(reply, "janus", json_string("janus_success"));
			json_object_set_new(reply, "session_id", json_integer(session->session_id));
			json_object_set_new(reply, "sender", json_integer(handle->handle_id));
			json_object_set_new(reply, "transaction", json_string("transaction_text"));
			json_t *plugin_data = json_object();
			json_object_set_new(plugin_data, "plugin", json_string(plugin_t->get_package()));
			json_object_set_new(plugin_data, "data", result->content);
			json_object_set_new(reply, "plugindata", plugin_data);
// Send the success reply
			//ret = janus_process_success(request, reply);
size_t size=json_dumpb(reply,NULL,0,0);
if(size==0){JANUS_LOG(LOG_WARN, "json_dumpb Size is null\n");json_decref(root);return;}
char*buf=alloca(size);
size=json_dumpb(reply,buf,size,0);
kore_websocket_send(ws_cl, 1, buf,size);
}
else if(result->type == JANUS_PLUGIN_OK_WAIT) {
//The plugin received the request but didn't process it yet, send an ack (asynchronous notifications may follow) */
json_auto_t *reply = json_object();
json_object_set_new(reply, "type", json_string("janus_ack"));
json_object_set_new(reply, "session_id", json_integer(session_id));
if(result->text)
json_object_set_new(reply, "hint", json_string(result->text));
json_object_set_new(reply, "transaction", json_string("transaction_text"));

//ret = janus_process_success(request, reply);

size_t size=json_dumpb(reply,NULL,0,0);
if(size==0){JANUS_LOG(LOG_WARN, "json_dumpb Size is null\n");json_decref(root);return;}
char*buf=alloca(size);
size=json_dumpb(reply,buf,size,0);
kore_websocket_send(ws_cl, 1, buf,size);
} 
else {
// Something went horribly wrong!
janus_process_e(ws_cl, root,"Plugin returned a severe (unknown) error");
janus_plugin_result_destroy(result);
goto jsondone;
}
janus_plugin_result_destroy(result);
jsondone:
	JANUS_LOG(LOG_WARN,"jsondone occured.");
	//json_decref(root);
} 	