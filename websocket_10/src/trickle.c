#include "helper.h"
// type trickle
void janus_process_er(struct connection*,json_t*,char*);
void janus_process_er(struct connection*ws_cl,json_t*root, char*grund){
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
void do_trickle(janus_ice_handle*handle,janus_session*session,json_t*root,guint64 session_id,struct connection*ws_cl)
 {
char *transak="janus_trans";
if(handle == NULL) {
// Trickle is an handle-level command
janus_process_er(ws_cl,root, "Unhandled request at this path: trickle. Handle is NULL");
//goto jsondone;
}
if(handle->app == NULL || handle->app_handle == NULL || !janus_plugin_session_is_alive(handle->app_handle)) {
janus_process_er(ws_cl,root,"No plugin to handle this trickle candidate");
//goto jsondone;
}
json_t *candidate = json_object_get(root, "candidate");
json_t *candidates = json_object_get(root, "candidates");
if(candidate == NULL && candidates == NULL) {
janus_process_er(ws_cl,root,"Missing mandatory element (candidate|candidates)");
//goto jsondone;
}
if(candidate != NULL && candidates != NULL) {
janus_process_er(ws_cl,root,"Can't have both candidate and candidates");
//goto jsondone;
}
if(janus_flags_is_set(&handle->webrtc_flags,JANUS_ICE_HANDLE_WEBRTC_CLEANING)) {
JANUS_LOG(LOG_ERR, "[%"SCNu64"] Received a trickle, but still cleaning a previous session\n", handle->handle_id);
janus_process_er(ws_cl,root,"Still cleaning a previous session");
//goto jsondone;
}
janus_mutex_lock(&handle->mutex);
if(!janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_TRICKLE)) {
//It looks like this peer supports Trickle, after all
JANUS_LOG(LOG_VERB, "Handle %"SCNu64" supports trickle even if it didn't negotiate it...\n", handle->handle_id);
janus_flags_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_TRICKLE);
}
//Is there any stream ready? this trickle may get here before the SDP it relates to 
if(handle->stream == NULL) {
JANUS_LOG(LOG_WARN, "[%"SCNu64"] No stream, queueing this trickle as it got here before the SDP...\n", handle->handle_id);
// Enqueue this trickle candidate(s), we'll process this later 
janus_ice_trickle *early_trickle = janus_ice_trickle_new(handle, transak, candidate ? candidate : candidates);
handle->pending_trickles = g_list_append(handle->pending_trickles, early_trickle);
//Send the ack right away, an event will tell the application if the candidate(s) failed 
goto trickledone;
}
// Is the ICE stack ready already? 
if(janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER) ||
				!janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_OFFER) ||
				!janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_ANSWER)) {
const char *cause = NULL;
if(janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER))
cause = "processing the offer";
else if(!janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_ANSWER))
cause = "waiting for the answer";
else if(!janus_flags_is_set(&handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_OFFER))
cause = "waiting for the offer";
JANUS_LOG(LOG_VERB, "[%"SCNu64"] Still %s, queueing this trickle to wait until we're done there...\n",handle->handle_id, cause);
// Enqueue this trickle candidate(s), we'll process this later 
janus_ice_trickle *early_trickle = janus_ice_trickle_new(handle, transak, candidate ? candidate : candidates);
handle->pending_trickles = g_list_append(handle->pending_trickles, early_trickle);
// Send the ack right away, an event will tell the application if the candidate(s) failed 
goto trickledone;
}
if(candidate != NULL) {
// We got a single candidate 
int error = 0;
const char *error_string = NULL;
if((error = janus_ice_trickle_parse(handle, candidate, &error_string)) != 0) {
kore_log(LOG_INFO,"error in trickle %s",error_string);
janus_process_er(ws_cl, root, "error_string");
janus_mutex_unlock(&handle->mutex);
//goto jsondone;
}
} else {
// We got multiple candidates in an array
if(!json_is_array(candidates)) {
janus_process_er(ws_cl, root,"candidates is not an array");
janus_mutex_unlock(&handle->mutex);
//goto jsondone;
}
JANUS_LOG(LOG_VERB, "Got multiple candidates (%zu)\n", json_array_size(candidates));
if(json_array_size(candidates) > 0) {
// Handle remote candidates 
size_t i = 0;
for(i=0; i<json_array_size(candidates); i++) {
json_t *c = json_array_get(candidates, i);
// FIXME We don't care if any trickle fails to parse 
janus_ice_trickle_parse(handle, c, NULL);
}
}
}

trickledone:
janus_mutex_unlock(&handle->mutex);
// We reply right away, not to block the web server... 
json_auto_t *reply = json_object();
json_object_set_new(reply, "type", json_string("janus_ack"));
json_object_set_new(reply, "session_id", json_integer(session_id));
json_object_set_new(reply, "transaction", json_string(transak));
// Send the success reply 
//ret = janus_process_success(request, reply);
size_t size=json_dumpb(reply,NULL,0,0);
if(size==0){JANUS_LOG(LOG_WARN, "json_dumpb Size is null\n");json_decref(root);return;}
char*buf=alloca(size);
size=json_dumpb(reply,buf,size,0);
kore_websocket_send(ws_cl, 1, buf,size);

} 
	
	
	
	