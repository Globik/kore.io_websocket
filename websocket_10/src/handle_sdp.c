#include "helper.h"

json_t *janus_plugin_handle_sdp(janus_plugin_session *plugin_session, janus_plugin *plugin,
								const char *sdp_type, const char *sdp, gboolean restart) {
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
		/* This is an offer from a plugin */
		offer = 1;
		janus_flags_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_OFFER);
		janus_flags_clear(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_ANSWER);
	} else if(!strcasecmp(sdp_type, "answer")) {
		/* This is an answer from a plugin */
		janus_flags_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_GOT_ANSWER);
	} else {
		/* TODO Handle other messages */
		JANUS_LOG(LOG_ERR, "Unknown type '%s'\n", sdp_type);
		return NULL;
	}
	/* Is this valid SDP? */
	char error_str[512];
	int audio = 0, video = 0, data = 0;
	janus_sdp *parsed_sdp = janus_sdp_preparse(sdp, error_str, sizeof(error_str), &audio, &video, &data);
	if(parsed_sdp == NULL) {
		JANUS_LOG(LOG_ERR, "[%"SCNu64"] Couldn't parse SDP... %s\n", ice_handle->handle_id, error_str);
		return NULL;
	}
	gboolean updating = FALSE;
	if(offer) {
		/* We may still not have a local ICE setup */
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
		/* Are we still cleaning up from a previous media session? */
		if(janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_CLEANING)) {
			JANUS_LOG(LOG_VERB, "[%"SCNu64"] Still cleaning up from a previous media session, let's wait a bit...\n", ice_handle->handle_id);
			gint64 waited = 0;
			while(janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_CLEANING)) {
				JANUS_LOG(LOG_VERB, "[%"SCNu64"] Still cleaning up from a previous media session, let's wait a bit...\n", ice_handle->handle_id);
				g_usleep(100000);
				waited += 100000;
				if(waited >= 3*G_USEC_PER_SEC) {
					JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- Waited 3 seconds, that's enough!\n", ice_handle->handle_id);
					JANUS_LOG(LOG_ERR, "[%"SCNu64"] Still cleaning a previous session\n", ice_handle->handle_id);
					janus_sdp_free(parsed_sdp);
					return NULL;
				}
			}
		}
		if(ice_handle->agent == NULL) {
			if(janus_is_rfc4588_enabled()) {
				/* We still need to configure the WebRTC stuff: negotiate RFC4588 by default */
				janus_flags_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_RFC4588_RTX);
			}
			/* Process SDP in order to setup ICE locally (this is going to result in an answer from the browser) */
			if(janus_ice_setup_local(ice_handle, 0, audio, video, data, 1) < 0) {
				JANUS_LOG(LOG_ERR, "[%"SCNu64"] Error setting ICE locally\n", ice_handle->handle_id);
				janus_sdp_free(parsed_sdp);
				return NULL;
			}
		} else {
			updating = TRUE;
			JANUS_LOG(LOG_INFO, "[%"SCNu64"] Updating existing session\n", ice_handle->handle_id);
		}
	} else {
		/* Check if transport wide CC is supported */
		int transport_wide_cc_ext_id = janus_rtp_header_extension_get_id(sdp, JANUS_RTP_EXTMAP_TRANSPORT_WIDE_CC);
		ice_handle->stream->do_transport_wide_cc = TRUE;
		ice_handle->stream->transport_wide_cc_ext_id = transport_wide_cc_ext_id;
	}
	if(!updating) {
		/* Wait for candidates-done callback */
		while(ice_handle->cdone < 1) {
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
	/* Anonymize SDP */
	if(janus_sdp_anonymize(parsed_sdp) < 0) {
		/* Invalid SDP */
		JANUS_LOG(LOG_ERR, "[%"SCNu64"] Invalid SDP\n", ice_handle->handle_id);
		janus_sdp_free(parsed_sdp);
		return NULL;
	}
	/* Check if this is a renegotiation and we need an ICE restart */
	if(offer && restart)
		janus_ice_restart(ice_handle);
	/* Add our details */
	janus_ice_stream *stream = ice_handle->stream;
	if(janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_RFC4588_RTX) &&
			stream && stream->rtx_payload_types == NULL) {
		/* Make sure we have a list of rtx payload types to generate, if needed */
		janus_sdp_mline *m = janus_sdp_mline_find(parsed_sdp, JANUS_SDP_VIDEO);
		if(m && m->ptypes) {
			stream->rtx_payload_types = g_hash_table_new(NULL, NULL);
			GList *ptypes = g_list_copy(m->ptypes), *tempP = ptypes;
			GList *rtx_ptypes = g_hash_table_get_values(stream->rtx_payload_types);
			while(tempP) {
				int ptype = GPOINTER_TO_INT(tempP->data);
				int rtx_ptype = ptype+1;
				while(g_list_find(m->ptypes, GINT_TO_POINTER(rtx_ptype))
						|| g_list_find(rtx_ptypes, GINT_TO_POINTER(rtx_ptype))) {
					rtx_ptype++;
					if(rtx_ptype > 127)
						rtx_ptype = 96;
					if(rtx_ptype == ptype) {
						/* We did a whole round? should never happen... */
						rtx_ptype = -1;
						break;
					}
				}
				if(rtx_ptype > 0)
					g_hash_table_insert(stream->rtx_payload_types, GINT_TO_POINTER(ptype), GINT_TO_POINTER(rtx_ptype));
				g_list_free(rtx_ptypes);
				rtx_ptypes = g_hash_table_get_values(stream->rtx_payload_types);
				tempP = tempP->next;
			}
			g_list_free(ptypes);
			g_list_free(rtx_ptypes);
		}
	}
	/* Enrich the SDP the plugin gave us with all the WebRTC related stuff */
	char *sdp_merged = janus_sdp_merge(ice_handle, parsed_sdp, offer ? TRUE : FALSE);
	if(sdp_merged == NULL) {
		/* Couldn't merge SDP */
		JANUS_LOG(LOG_ERR, "[%"SCNu64"] Error merging SDP\n", ice_handle->handle_id);
		janus_sdp_free(parsed_sdp);
		return NULL;
	}
	janus_sdp_free(parsed_sdp);

	if(!updating) {
		if(offer) {
			/* We set the flag to wait for an answer before handling trickle candidates */
			janus_flags_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
		} else {
			JANUS_LOG(LOG_VERB, "[%"SCNu64"] Done! Ready to setup remote candidates and send connectivity checks...\n", ice_handle->handle_id);
			janus_mutex_lock(&ice_handle->mutex);
			/* We got our answer */
			janus_flags_clear(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_PROCESSING_OFFER);
			/* Any pending trickles? */
			if(ice_handle->pending_trickles) {
				JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- Processing %d pending trickle candidates\n", ice_handle->handle_id, g_list_length(ice_handle->pending_trickles));
				GList *temp = NULL;
				while(ice_handle->pending_trickles) {
					temp = g_list_first(ice_handle->pending_trickles);
					ice_handle->pending_trickles = g_list_remove_link(ice_handle->pending_trickles, temp);
					janus_ice_trickle *trickle = (janus_ice_trickle *)temp->data;
					g_list_free(temp);
					if(trickle == NULL)
						continue;
					if((janus_get_monotonic_time() - trickle->received) > 45*G_USEC_PER_SEC) {
						/* FIXME Candidate is too old, discard it */
						janus_ice_trickle_destroy(trickle);
						/* FIXME We should report that */
						continue;
					}
					json_t *candidate = trickle->candidate;
					if(candidate == NULL) {
						janus_ice_trickle_destroy(trickle);
						continue;
					}
					if(json_is_object(candidate)) {
						/* We got a single candidate */
						int error = 0;
						const char *error_string = NULL;
						if((error = janus_ice_trickle_parse(ice_handle, candidate, &error_string)) != 0) {
							/* FIXME We should report the error parsing the trickle candidate */
						}
					} else if(json_is_array(candidate)) {
						/* We got multiple candidates in an array */
						JANUS_LOG(LOG_VERB, "[%"SCNu64"] Got multiple candidates (%zu)\n", ice_handle->handle_id, json_array_size(candidate));
						if(json_array_size(candidate) > 0) {
							/* Handle remote candidates */
							size_t i = 0;
							for(i=0; i<json_array_size(candidate); i++) {
								json_t *c = json_array_get(candidate, i);
								/* FIXME We don't care if any trickle fails to parse */
								janus_ice_trickle_parse(ice_handle, c, NULL);
							}
						}
					}
					/* Done, free candidate */
					janus_ice_trickle_destroy(trickle);
				}
			}
			/* This was an answer, check if it's time to start ICE */
			if(janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_TRICKLE) &&
					!janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_ALL_TRICKLES)) {
				JANUS_LOG(LOG_VERB, "[%"SCNu64"]   -- ICE Trickling is supported by the browser, waiting for remote candidates...\n", ice_handle->handle_id);
				janus_flags_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_START);
			} else {
				JANUS_LOG(LOG_VERB, "[%"SCNu64"] Done! Sending connectivity checks...\n", ice_handle->handle_id);
				if(ice_handle->stream_id > 0) {
					janus_ice_setup_remote_candidates(ice_handle, ice_handle->stream_id, 1);
				}
			}
			janus_mutex_unlock(&ice_handle->mutex);
		}
	}
#ifdef HAVE_SCTP
	if(!offer && janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_READY)) {
		/* Renegotiation: check if datachannels were just added on an existing PeerConnection */
		if(janus_flags_is_set(&ice_handle->webrtc_flags, JANUS_ICE_HANDLE_WEBRTC_DATA_CHANNELS)) {
			janus_ice_stream *stream = ice_handle->stream;
			if(stream != NULL && stream->component != NULL &&
					stream->component->dtls != NULL && stream->component->dtls->sctp == NULL) {
				/* Create SCTP association as well */
				JANUS_LOG(LOG_WARN, "[%"SCNu64"] Creating datachannels...\n", ice_handle->handle_id);
				janus_dtls_srtp_create_sctp(stream->component->dtls);
			}
		}
	}
#endif

	/* Prepare JSON event */
	json_t *jsep = json_object();
	json_object_set_new(jsep, "type", json_string(sdp_type));
	json_object_set_new(jsep, "sdp", json_string(sdp_merged));
	char *tmp = ice_handle->local_sdp;
	ice_handle->local_sdp = sdp_merged;
	g_free(tmp);
	return jsep;
}



