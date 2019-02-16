# webRTC peer_connection based on rawrtc

### console

```

[parent]: running on http://127.0.0.1:8889
ignoring "tls_dhparam" on line 6
ignoring "certfile" on line 12
ignoring "certkey" on line 13
[parent]: privsep: no root path set, using working directory
[parent]: privsep: will not change user
[parent]: privsep: will not chroot
[parent]: kore is starting up
[parent]: pgsql built-in enabled
[parent]: tasks built-in enabled
[wrk 0]: worker 0 started (cpu#0)
state init!
[wrk 0]: websocket connected , path /connect 0xb6651f40
[wrk 0]: websocket connected 0xb65f2410
[wrk 0]: websocket message length 67
[wrk 0]: type msg
[wrk 0]: websocket message length 27
[wrk 0]: Start rawrtc client
[wrk 0]: *** a dedicated thread! Not to store! ***
[wrk 0]: task_thread: #129 starting
[wrk 0]: task_thread#129: woke up
[wrk 0]: task_thread#129: executing 0xb77774a0
[wrk 0]: LEN: 5
argc: 3
mi.arvi[0] peer-connection
mi.argvi[1] 1
mi.argvi[2] host
[000000000] peer-connection-app: ICE ROLE IS 1
[000000000] peer-connection-app: argc>= 3!
Create peer connection
[000000642] peer-connection-app: negotiation_needed_handler
[000000642] helper-handler: (A) Negotiation needed
[000000643] helper-handler: (A) ICE gatherer state: gathering
[000000643] (A) ICE candidate: foundation=0a2223cf, protocol=udp, priority=1, ip=10.34.35.207, port=60042, type=host, tcp-type=n/a, related-address=n/a, related-port=0; URL: n/a; mid=rawrtc-sctp-dc, media_line_index=0, username_fragment=jSK7ak4U63ZcxgIO; enabled
[000000644] helper-common: (A) ICE gatherer last local candidate
!candidate in local_candidate_handler
[000000644] peer-connection-app: Local Description:
{"type":"offer","sdp":"v=0\r\no=sdpartanic-rawrtc-0.2.2 3457381168 1 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=ice-options:trickle\r\na=group:BUNDLE rawrtc-sctp-dc\r\nm=application 9 DTLS\/SCTP 5000\r\nc=IN IP4 0.0.0.0\r\na=mid:rawrtc-sctp-dc\r\na=sendrecv\r\na=ice-ufrag:jSK7ak4U63ZcxgIO\r\na=ice-pwd:0VKG3HGGSJO8NsuSoDuGWhz9b14J5wai\r\na=setup:actpass\r\na=fingerprint:sha-256 DA:12:D5:56:69:36:59:57:31:D5:EC:D5:62:ED:3C:32:6F:66:04:F6:30:7C:40:FB:03:0D:D3:60:C1:F7:D9:3C\r\na=tls-id:pw59BKPgFifRseL2AAgBWjNl6Do9CnV8\r\na=sctpmap:5000 webrtc-datachannel 65535\r\na=max-message-size:0\r\na=candidate:0a2223cf 1 udp 1 10.34.35.207 60042 typ host\r\na=end-of-candidates\r\n"}
671
client->task!
[wrk 0]: LEN: 671
[000000657] helper-handler: (A) ICE gatherer state: complete
[000000658] helper-handler: (A) Signaling state change: have-local-offer
***Start main loop***
[wrk 0]: websocket message length 614
[wrk 0]: webRTC answer
[wrk 0]: mqueue_handler occured
[wrk 0]: f==2, parse remote description
[000000728] peer-connection-app: my_parse_remote_description entering
[000000729] peer-connection-app: Applying remote description
[000000729] helper-handler: (A) ICE transport state: checking
[000000729] helper-handler: (A) Peer connection state change: connecting
[000000753] helper-handler: (A) Signaling state change: stable
[000000760] helper-handler: (A) ICE transport state: connected
[000000797] helper-handler: (A) Peer connection state change: connected
[000000805] peer-connection-app: data_channel_open_handler
[000000805] helper-handler: (A) Data channel open: cat-noises

[000003946] helper-handler: (A) Incoming message for data channel cat-noises: 12 bytes
[000003946] peer-connection-app: *** MY!!! (A) sending 12 bytes ***
[000003957] helper-handler: (A) Data channel buffered amount low: cat-noises
^C[parent]: server shutting down
[parent]: waiting for workers to drain and shutdown
[wrk 0]: teardown
[wrk 0]: mqueue push 0
[wrk 0]: mqueue_handler occured
[wrk 0]: F: 1
*** Stop client & bye ***
[000026946] helper-handler: (A) Signaling state change: closed
[000026946] helper-handler: (A) Peer connection state change: closed
[000026946] helper-handler: (A) Data channel closed: cat-noises
[wrk 0]: ***bye from a task!***
[wrk 0]: before kore_task_finish
*** after mutex lock ***
[wrk 0]: parent gone, shutting down
[wrk 0]: websocket disconnected 0xb663f410
[parent]: worker 0 (1018)-> status 0
[parent]: goodbye

```
### bug?

without turn and stun routine re_main loop not starts and peer connection set parameters to quickly and no datachannel in frontentd opened
as a result.
