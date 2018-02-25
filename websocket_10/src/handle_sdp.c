#include "helper.h"

json_t *janus_plugin_handle_sdp(janus_plugin_session *plugin_session, janus_plugin *plugin, const char *sdp_type, const char *sdp) {
	if(!plugin_session || plugin_session < (janus_plugin_session *)0x1000 ||
			!janus_plugin_session_is_alive(plugin_session) || plugin_session->stopped ||
			plugin == NULL || sdp_type == NULL || sdp == NULL) {
		JANUS_LOG(LOG_ERR, "Invalid arguments\n");
		return NULL;
	}
	janus_ice_handle *ice_handle = (janus_ice_handle *)plugin_session->gateway_handle;
	//~ if(ice_handle == NULL || janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_READY)) {
	if(ice_handle == NULL) {
		JANUS_LOG(LOG_ERR, "Invalid ICE handle\n");
		return NULL;
	}
	int offer = 0;
	if(!strcasecmp(sdp_type, "offer")) {
		// This is an offer from a plugin 
		offer = 1;
		janus_flags_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_OFFER);
		janus_flags_clear(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_ANSWER);
	} else if(!strcasecmp(sdp_type, "answer")) {
		// This is an answer from a plugin 
		janus_flags_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_ANSWER);
	} else {
		// TODO Handle other messages 
		JANUS_LOG(LOG_ERR, "Unknown type '%s'\n", sdp_type);
		return NULL;
	}
	// Is this valid SDP? 
	char error_str[512];
	int audio = 0, video = 0, data = 0, bundle = 0, rtcpmux = 0, trickle = 0;
	janus_sdp *parsed_sdp = janus_sdp_preparse(sdp, error_str, sizeof(error_str), &audio, &video, &data, &bundle, &rtcpmux, &trickle);
	if(parsed_sdp == NULL) {
		JANUS_LOG(LOG_ERR, "[%"SCNu64"] Couldn't parse SDP... %s\n", ice_handle->handle_id, error_str);
		return NULL;
	}
	gboolean updating = FALSE;
	if(offer) {
		// We still don't have a local ICE setup 
		JANUS_LOG(LOG_VERB, "[%"SCNu64"] Audio %s been negotiated\n", ice_handle->handle_id, audio ? "has" : "has NOT");
		if(audio > 1) {
			JANUS_LOG(LOG_ERR, "[%"SCNu64"] More than one audio line? only going to negotiate one...\n", ice_handle->handle_id);
		}
		JANUS_LOG(LOG_VERB, "[%"SCNu64"] Video %s been negotiated\n", ice_handle->handle_id, video ? "has" : "has NOT");
		if(video > 1) {
			JANUS_LOG(LOG_ERR, "[%"SCNu64"] More than one video line? only going to negotiate one...\n", ice_handle->handle_id);
		}
		JANUS_LOG(LOG_VERB, "[%"SCNu64"] SCTP/DataChannels %s been negotiated\n", ice_handle->handle_id, data ? "have" : "have NOT");
		if(data > 1) {
			JANUS_LOG(LOG_ERR, "[%"SCNu64"] More than one data line? only going to negotiate one...\n", ice_handle->handle_id);
		}
#ifndef HAVE_SCTP
		if(data) {
			JANUS_LOG(LOG_WARN, "[%"SCNu64"]   -- DataChannels have been negotiated, but support for them has not been compiled...\n", ice_handle->handle_id);
		}
#endif
		// Are we still cleaning up from a previous media session? 
		if(janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_CLEANING)) {
			JANUS_LOG(LOG_VERB, "[%"SCNu64"] Still cleaning up from a previous media session, let's wait a bit...\n", ice_handle->handle_id);
			gint64 waited = 0;
			while(janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_CLEANING)) {
				JANUS_LOG(LOG_VERB, "[%"SCNu64"] Still cleaning up from a previous media session, let's wait a bit...\n", ice_handle->handle_id);
				g_usleep(100000);
				waited += 100000;
				if(waited >= 3*G_USEC_PER_SEC) {
					JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- Waited 3 seconds, that's enough!\n", ice_handle->handle_id);
					break;
				}
			}
		}
		if(ice_handle->agent == NULL) {
			// Process SDP in order to setup ICE locally (this is going to result in an answer from the browser) 
			if(janus_ice_setup_local(ice_handle, 0, audio, video, data, bundle, rtcpmux, trickle) < 0) {
				JANUS_LOG(LOG_ERR, "[%"SCNu64"] Error setting ICE locally\n", ice_handle->handle_id);
				janus_sdp_free(parsed_sdp);
				return NULL;
			}
		} else {
			updating = TRUE;
			JANUS_LOG(LOG_INFO, "[%"SCNu64"] Updating existing session\n", ice_handle->handle_id);
		}
	}
	if(!updating) {
		// Wait for candidates-done callback 
		while(ice_handle->cdone < ice_handle->streams_num) {
			if(ice_handle == NULL || janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_STOP)
					|| janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ALERT)) {
				JANUS_LOG(LOG_WARN, "[%"SCNu64"] Handle detached or PC closed, giving up...!\n", ice_handle ? ice_handle->handle_id : 0);
				janus_sdp_free(parsed_sdp);
				return NULL;
			}
			JANUS_LOG(LOG_VERB, "[%"SCNu64"] Waiting for candidates-done callback...\n", ice_handle->handle_id);
			g_usleep(100000);
			if(ice_handle->cdone < 0) {
				JANUS_LOG(LOG_ERR, "[%"SCNu64"] Error gathering candidates!\n", ice_handle->handle_id);
				janus_sdp_free(parsed_sdp);
				return NULL;
			}
		}
	}
	// Anonymize SDP 
	if(janus_sdp_anonymize(parsed_sdp) < 0) {
		// Invalid SDP 
		JANUS_LOG(LOG_ERR, "[%"SCNu64"] Invalid SDP\n", ice_handle->handle_id);
		janus_sdp_free(parsed_sdp);
		return NULL;
	}
	// Add our details 
	char *sdp_merged = janus_sdp_merge(ice_handle, parsed_sdp);
	if(sdp_merged == NULL) {
		// Couldn't merge SDP 
		JANUS_LOG(LOG_ERR, "[%"SCNu64"] Error merging SDP\n", ice_handle->handle_id);
		janus_sdp_free(parsed_sdp);
		return NULL;
	}
	janus_sdp_free(parsed_sdp);
	// FIXME Any disabled m-line? 
	if(strstr(sdp_merged, "m=audio 0")) {
		JANUS_LOG(LOG_VERB, "[%"SCNu64"] Audio disabled via SDP\n", ice_handle->handle_id);
		if(!janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_BUNDLE)
				|| (!video && !data)) {
			JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- Marking audio stream as disabled\n", ice_handle->handle_id);
			janus_ice_stream *stream = g_hash_table_lookup(ice_handle->streams, GUINT_TO_POINTER(ice_handle->audio_id));
			if(stream)
				stream->disabled = TRUE;
		}
	}
	if(strstr(sdp_merged, "m=video 0")) {
		JANUS_LOG(LOG_VERB, "[%"SCNu64"] Video disabled via SDP\n", ice_handle->handle_id);
		if(!janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_BUNDLE)
				|| (!audio && !data)) {
			JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- Marking video stream as disabled\n", ice_handle->handle_id);
			janus_ice_stream *stream = NULL;
			if(!janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_BUNDLE)) {
				stream = g_hash_table_lookup(ice_handle->streams, GUINT_TO_POINTER(ice_handle->video_id));
			} else {
				gint id = ice_handle->audio_id > 0 ? ice_handle->audio_id : ice_handle->video_id;
				stream = g_hash_table_lookup(ice_handle->streams, GUINT_TO_POINTER(id));
			}
			if(stream)
				stream->disabled = TRUE;
		}
	}
	if(strstr(sdp_merged, "m=application 0 DTLS/SCTP")) {
		JANUS_LOG(LOG_VERB, "[%"SCNu64"] Data Channel disabled via SDP\n", ice_handle->handle_id);
		if(!janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_BUNDLE)
				|| (!audio && !video)) {
			JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- Marking data channel stream as disabled\n", ice_handle->handle_id);
			janus_ice_stream *stream = NULL;
			if(!janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_BUNDLE)) {
				stream = g_hash_table_lookup(ice_handle->streams, GUINT_TO_POINTER(ice_handle->data_id));
			} else {
				gint id = ice_handle->audio_id > 0 ? ice_handle->audio_id : (ice_handle->video_id > 0 ? ice_handle->video_id : ice_handle->data_id);
				stream = g_hash_table_lookup(ice_handle->streams, GUINT_TO_POINTER(id));
			}
			if(stream)
				stream->disabled = TRUE;
		}
	}

	if(!updating) {
		if(offer) {
			// We set the flag to wait for an answer before handling trickle candidates 
			janus_flags_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
		} else {
			JANUS_LOG(LOG_VERB, "[%"SCNu64"] Done! Ready to setup remote candidates and send connectivity checks...\n", ice_handle->handle_id);
			if(janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_BUNDLE)) {
				JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- bundle is supported by the browser, getting rid of one of the RTP/RTCP components, if any...\n", ice_handle->handle_id);
				if(audio) {
					// Get rid of video and data, if present 
					if(ice_handle->streams && ice_handle->video_stream) {
						if(ice_handle->audio_stream->rtp_component && ice_handle->video_stream->rtp_component)
							ice_handle->audio_stream->rtp_component->do_video_nacks = ice_handle->video_stream->rtp_component->do_video_nacks;
						ice_handle->audio_stream->video_ssrc = ice_handle->video_stream->video_ssrc;
						ice_handle->audio_stream->video_ssrc_peer = ice_handle->video_stream->video_ssrc_peer;
						ice_handle->audio_stream->video_ssrc_peer_rtx = ice_handle->video_stream->video_ssrc_peer_rtx;
						ice_handle->audio_stream->video_ssrc_peer_sim_1 = ice_handle->video_stream->video_ssrc_peer_sim_1;
						ice_handle->audio_stream->video_ssrc_peer_sim_2 = ice_handle->video_stream->video_ssrc_peer_sim_2;
						nice_agent_attach_recv(ice_handle->agent, ice_handle->video_stream->stream_id, 1, g_main_loop_get_context (ice_handle->iceloop), NULL, NULL);
						if(!ice_handle->force_rtcp_mux && !janus_ice_is_rtcpmux_forced())
							nice_agent_attach_recv(ice_handle->agent, ice_handle->video_stream->stream_id, 2, g_main_loop_get_context (ice_handle->iceloop), NULL, NULL);
						nice_agent_remove_stream(ice_handle->agent, ice_handle->video_stream->stream_id);
						janus_ice_stream_free(ice_handle->streams, ice_handle->video_stream);
					}
					ice_handle->video_stream = NULL;
					ice_handle->video_id = 0;
					if(ice_handle->streams && ice_handle->data_stream) {
						nice_agent_attach_recv(ice_handle->agent, ice_handle->data_stream->stream_id, 1, g_main_loop_get_context (ice_handle->iceloop), NULL, NULL);
						nice_agent_remove_stream(ice_handle->agent, ice_handle->data_stream->stream_id);
						janus_ice_stream_free(ice_handle->streams, ice_handle->data_stream);
					}
					ice_handle->data_stream = NULL;
					ice_handle->data_id = 0;
					if(!video) {
						if(ice_handle->audio_stream->rtp_component)
							ice_handle->audio_stream->rtp_component->do_video_nacks = FALSE;
						ice_handle->audio_stream->video_ssrc = 0;
						ice_handle->audio_stream->video_ssrc_peer = 0;
						g_free(ice_handle->audio_stream->video_rtcp_ctx);
						ice_handle->audio_stream->video_rtcp_ctx = NULL;
					}
				} else if(video) {
//Get rid of data, if present 
if(ice_handle->streams && ice_handle->data_stream) {
nice_agent_attach_recv(ice_handle->agent, ice_handle->data_stream->stream_id, 1, g_main_loop_get_context (ice_handle->iceloop), NULL, NULL);
nice_agent_remove_stream(ice_handle->agent, ice_handle->data_stream->stream_id);
janus_ice_stream_free(ice_handle->streams, ice_handle->data_stream);
}
ice_handle->data_stream = NULL;
ice_handle->data_id = 0;
}
}
if(janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_RTCPMUX) && !ice_handle->force_rtcp_mux && !janus_ice_is_rtcpmux_forced()) {
JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- rtcp-mux is supported by the browser, getting rid of RTCP components, if any...\n", ice_handle->handle_id);
if(ice_handle->audio_stream && ice_handle->audio_stream->rtcp_component && ice_handle->audio_stream->components != NULL) {
nice_agent_attach_recv(ice_handle->agent, ice_handle->audio_id, 2, g_main_loop_get_context (ice_handle->iceloop), NULL, NULL);
// Free the component 
janus_ice_component_free(ice_handle->audio_stream->components, ice_handle->audio_stream->rtcp_component);
ice_handle->audio_stream->rtcp_component = NULL;
// Create a dummy candidate and enforce it as the one to use for this now unneeded component 
NiceCandidate *c = nice_candidate_new(NICE_CANDIDATE_TYPE_HOST);
			c->component_id = 2;
					c->stream_id = ice_handle->audio_stream->stream_id;
#ifndef HAVE_LIBNICE_TCP
					c->transport = NICE_CANDIDATE_TRANSPORT_UDP;
#endif
					strncpy(c->foundation, "1", NICE_CANDIDATE_MAX_FOUNDATION);
					c->priority = 1;
					nice_address_set_from_string(&c->addr, "127.0.0.1");
					nice_address_set_port(&c->addr, janus_ice_get_rtcpmux_blackhole_port());
					c->username = g_strdup(ice_handle->audio_stream->ruser);
					c->password = g_strdup(ice_handle->audio_stream->rpass);
					if(!nice_agent_set_selected_remote_candidate(ice_handle->agent, ice_handle->audio_stream->stream_id, 2, c)) {
//JANUS_LOG(LOG_ERR, "[%"SCNu64"] Error forcing dummy candidate on RTCP component of stream %d\n",
		//ice_handle->handle_id, ice_handle->audio_stream->stream_id);
						nice_candidate_free(c);
					}
				}
if(ice_handle->video_stream && ice_handle->video_stream->rtcp_component && ice_handle->video_stream->components != NULL) {
nice_agent_attach_recv(ice_handle->agent, ice_handle->video_id, 2, g_main_loop_get_context (ice_handle->iceloop), NULL, NULL);
					// Free the component 
janus_ice_component_free(ice_handle->video_stream->components, ice_handle->video_stream->rtcp_component);
ice_handle->video_stream->rtcp_component = NULL;
					// Create a dummy candidate and enforce it as the one to use for this now unneeded component 
NiceCandidate *c = nice_candidate_new(NICE_CANDIDATE_TYPE_HOST);
					c->component_id = 2;
					c->stream_id = ice_handle->video_stream->stream_id;
#ifndef HAVE_LIBNICE_TCP
					c->transport = NICE_CANDIDATE_TRANSPORT_UDP;
#endif
strncpy(c->foundation, "1", NICE_CANDIDATE_MAX_FOUNDATION);
c->priority = 1;
nice_address_set_from_string(&c->addr, "127.0.0.1");
nice_address_set_port(&c->addr, janus_ice_get_rtcpmux_blackhole_port());
c->username = g_strdup(ice_handle->video_stream->ruser);
c->password = g_strdup(ice_handle->video_stream->rpass);
if(!nice_agent_set_selected_remote_candidate(ice_handle->agent, ice_handle->video_stream->stream_id, 2, c)) {
//JANUS_LOG(LOG_ERR, "[%"SCNu64"] Error forcing dummy candidate on RTCP component of stream %d\n",
//ice_handle->handle_id, ice_handle->video_stream->stream_id);
nice_candidate_free(c);
}
}
}
janus_mutex_lock(&ice_handle->mutex);
			// We got our answer 
janus_flags_clear(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
			// Any pending trickles? 
if(ice_handle->pending_trickles) {
//JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- Processing %d pending trickle candidates\n", 
//		  ice_handle->handle_id, g_list_length(ice_handle->pending_trickles));
GList *temp = NULL;
while(ice_handle->pending_trickles) {
temp = g_list_first(ice_handle->pending_trickles);
ice_handle->pending_trickles = g_list_remove_link(ice_handle->pending_trickles, temp);
janus_ice_trickle *trickle = (janus_ice_trickle *)temp->data;
					g_list_free(temp);
					if(trickle == NULL)
						continue;
					if((janus_get_monotonic_time() - trickle->received) > 45*G_USEC_PER_SEC) {
						// FIXME Candidate is too old, discard it 
						janus_ice_trickle_destroy(trickle);
						// FIXME We should report that 
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
						if((error = janus_ice_trickle_parse(ice_handle, candidate, &error_string)) != 0) {
							// FIXME We should report the error parsing the trickle candidate 
						}
					} else if(json_is_array(candidate)) {
						//We got multiple candidates in an array 
						JANUS_LOG(LOG_VERB, "[%"SCNu64"] Got multiple candidates (%zu)\n", ice_handle->handle_id, json_array_size(candidate));
						if(json_array_size(candidate) > 0) {
							//Handle remote candidates 
							size_t i = 0;
							for(i=0; i<json_array_size(candidate); i++) {
								json_t *c = json_array_get(candidate, i);
								//FIXME We don't care if any trickle fails to parse 
								janus_ice_trickle_parse(ice_handle, c, NULL);
							}
						}
					}
					// Done, free candidate 
					janus_ice_trickle_destroy(trickle);
				}
			}
			// This was an answer, check if it's time to start ICE 
			if(janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_TRICKLE) &&
					!janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ALL_TRICKLES)) {
				JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- ICE Trickling is supported by the browser, waiting for remote candidates...\n", ice_handle->handle_id);
				janus_flags_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_START);
			} else {
				JANUS_LOG(LOG_VERB, "[%"SCNu64"] Done! Sending connectivity checks...\n", ice_handle->handle_id);
				if(ice_handle->audio_id > 0) {
					janus_ice_setup_remote_candidates(ice_handle, ice_handle->audio_id, 1);
					if(!janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_RTCPMUX))	// http://tools.ietf.org/html/rfc5761#section-5.1.3 
						janus_ice_setup_remote_candidates(ice_handle, ice_handle->audio_id, 2);
				}
				if(ice_handle->video_id > 0) {
					janus_ice_setup_remote_candidates(ice_handle, ice_handle->video_id, 1);
					if(!janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_RTCPMUX))	//http://tools.ietf.org/html/rfc5761#section-5.1.3 
						janus_ice_setup_remote_candidates(ice_handle, ice_handle->video_id, 2);
				}
if(ice_handle->data_id > 0) {
janus_ice_setup_remote_candidates(ice_handle, ice_handle->data_id, 1);
}
}
janus_mutex_unlock(&ice_handle->mutex);
}
}

	// Prepare JSON event 
json_t *jsep = json_object();
json_object_set_new(jsep, "type", json_string(sdp_type));
json_object_set_new(jsep, "sdp", json_string(sdp_merged));
ice_handle->local_sdp = sdp_merged;
return jsep;
}


