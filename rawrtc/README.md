# RAWRTC works from within  the kore.c framework

## peer-connection.c

[RAWRTC](https://github.com/rawrtc/rawrtc) - A WebRTC and ORTC library with a small footprint that runs everywhere

Taken from the tools [peer-connection.c](https://github.com/rawrtc/rawrtc/blob/master/src/tools/peer-connection.c)

In a dedicated thread task. As a proof of concept.



```

globik@globik-laptop:~/kore.io_websocket/rawrtc$ kodev run
building rawrtc (dev)
CFLAGS=-Wall -Wmissing-declarations -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Wcast-qual -Wsign-compare -fPIC -Isrc -Isrc/includes -I/usr/local/include -DKORE_NO_TLS -DKORE_USE_PGSQL -DKORE_USE_TASKS -I/usr/include/postgresql -g -D_GNU_SOURCE -DDEBUG=1 -g -I/home/globik/rawrtc/build/prefix/include -I/home/globik/rawrtc/build/prefix/include/re -I/home/globik/rawrtc/build/prefix/include/rew -I/home/globik/rawrtc/src/tools 
LDFLAGS=-shared -Wl,-rpath /home/globik/rawrtc/build/src/tools/helper -L /home/globik/rawrtc/build/src/tools/helper -lrawrtc-helper -pthread -Wl,-rpath /home/globik/rawrtc/build/prefix/lib -L/home/globik/rawrtc/build/prefix/lib -lusrsctp -lre -lrew -lcrypto -lssl -lrawrtc 
nothing to be done!
[parent]: running on http://127.0.0.1:8888
ignoring "tls_dhparam" on line 6
ignoring "certfile" on line 10
ignoring "certkey" on line 11
[parent]: privsep: no root path set, using working directory
[parent]: privsep: will not change user
[parent]: privsep: will not chroot
[parent]: kore is starting up
[parent]: pgsql built-in enabled
[parent]: tasks built-in enabled
[wrk 0]: worker 0 started (cpu#0)
state init!
[wrk 0]: *** a dedicated thread! Not to store! ***
[wrk 0]: task_thread: #0 starting
[wrk 0]: task_thread#0: woke up
[wrk 0]: task_thread#0: executing 0xb6a81460
[wrk 0]: Task msg: mama
LEN: 5
argc: 3
mi.arvi[0] peer-connection
mi.argvi[1] 1
mi.argvi[2] host
[000000000] peer-connection-app: SSSSSSSSSSSSSSSS Init
[000000001] peer-connection-app: ICE ROLE IS 1
[000000001] peer-connection-app: argc>= 3!
[000000590] helper-handler: (A) Negotiation needed
[000000590] helper-handler: (A) ICE gatherer state: gathering
[000000591] (A) ICE candidate: foundation=0a2235b9, protocol=udp, priority=1, ip=10.34.53.185, port=40914, type=host, tcp-type=n/a, related-address=n/a, related-port=0; URL: n/a; mid=rawrtc-sctp-dc, media_line_index=0, username_fragment=J0qwYvwbKp0jxlza; enabled
[000000592] helper-handler: (A) Signaling state change: have-local-offer
***Start main loop***
[000000737] (A) ICE candidate: foundation=d97653b4, protocol=udp, priority=1, ip=217.118.83.181, port=27070, type=srflx, tcp-type=n/a, related-address=10.34.53.185, related-port=40914; URL: stun:stun.l.google.com:19302; mid=rawrtc-sctp-dc, media_line_index=0, username_fragment=J0qwYvwbKp0jxlza; disabled
[000000767] ice-gatherer: TODO: Gather relay candidates using server 5.148.189.205:443 (turn:turn.threema.ch:443)
[000040190] ice-gatherer: STUN request failed, reason: Connection timed out
[000040190] helper-common: (A) ICE gatherer last local candidate
[000040190] peer-connection-app: Local Description:
{"type":"offer","sdp":"v=0\r\no=sdpartanic-rawrtc-0.2.2 3824215037 1 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=ice-options:trickle\r\na=group:BUNDLE rawrtc-sctp-dc\r\nm=application 9 DTLS\/SCTP 5000\r\nc=IN IP4 0.0.0.0\r\na=mid:rawrtc-sctp-dc\r\na=sendrecv\r\na=ice-ufrag:J0qwYvwbKp0jxlza\r\na=ice-pwd:vDbkovacQgHJACzkIyfFNReybetTc5BO\r\na=setup:actpass\r\na=fingerprint:sha-256 8C:09:4D:9F:ED:88:05:E2:97:89:4F:96:9D:17:84:54:13:CC:9D:77:03:B3:A0:C0:BA:84:75:96:8C:D1:60:D4\r\na=tls-id:PLjwVVcqiGSl9aMO9JGb0HHhNjvPXiuH\r\na=sctpmap:5000 webrtc-datachannel 65535\r\na=max-message-size:0\r\na=candidate:0a2235b9 1 udp 1 10.34.53.185 40914 typ host\r\na=candidate:d97653b4 1 udp 1 217.118.83.181 27070 typ srflx raddr 10.34.53.185 rport 40914\r\na=end-of-candidates\r\n"}
[000040204] helper-handler: (A) ICE gatherer state: complete
{"type":"answer","sdp":"v=0\r\no=- 180606819526618186 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=group:BUNDLE rawrtc-sctp-dc\r\na=msid-semantic: WMS\r\nm=application 34259 DTLS/SCTP 5000\r\nc=IN IP4 10.34.53.185\r\nb=AS:30\r\na=candidate:3948008002 1 udp 2113937151 10.34.53.185 34259 typ host generation 0 network-cost 50\r\na=ice-ufrag:C6sp\r\na=ice-pwd:usnUy9ktd9TB9cgJzeovN5WQ\r\na=ice-options:trickle\r\na=fingerprint:sha-256 D0:44:C3:3E:BC:D9:3B:A4:45:F4:69:EF:C8:8D:5F:7E:4A:DD:62:0D:0C:6E:C2:60:98:9A:1C:73:BD:B6:5B:F5\r\na=setup:active\r\na=mid:rawrtc-sctp-dc\r\na=sctpmap:5000 webrtc-datachannel 1024\r\n"}
[000078260] peer-connection-app: Applying remote description
[000078260] helper-handler: (A) ICE transport state: checking
[000078260] helper-handler: (A) Peer connection state change: connecting
[000078277] helper-handler: (A) Signaling state change: stable
[000078278] helper-handler: (A) ICE transport state: connected
[000078303] helper-handler: (A) Peer connection state change: connected
[000078353] helper-handler: (A) Data channel open: cat-noises
[000078353] peer-connection-app: (A) Sending 8192 bytes
[000078354] helper-handler: (A) Data channel open: bear-noises
[000078380] helper-handler: (A) Data channel buffered amount low: cat-noises
[000078380] helper-handler: (A) Data channel buffered amount low: bear-noises
[000078384] helper-handler: (A) Incoming message for data channel cat-noises: 16384 bytes
[000079335] helper-handler: (A) New data channel instance: dinosaur-noises
[000079347] helper-handler: (A) Data channel buffered amount low: cat-noises
[000079347] helper-handler: (A) Data channel buffered amount low: bear-noises
[000079366] sctp-transport: No message handler, message of 16384 bytes has been discarded
[000104690] ice-gatherer: STUN request failed, reason: Connection timed out
^C[parent]: server shutting down
[parent]: waiting for workers to drain and shutdown
[wrk 0]: teardown
[wrk 0]: mqueue push 0
[wrk 0]: mqueue_handler occured
[wrk 0]: F: 1
*** Stop client & bye ***
[000106283] helper-handler: (A) Signaling state change: closed
[000106283] helper-handler: (A) Peer connection state change: closed
[000106283] helper-handler: (A) Data channel closed: cat-noises
[000106283] helper-handler: (A) Data channel closed: bear-noises
Timers (1):
  0xb6a81400: th=0xb6a6a170 expire=2068ms
[wrk 0]: ***bye from a task!***
[wrk 0]: before kore_task_finish
*** after mutex lock ***
[wrk 0]: parent gone, shutting down
[parent]: worker 0 (29779)-> status 0
[parent]: goodbye
globik@globik-laptop:~/kore.io_websocket/rawrtc$ 

```

## test


1.  run ./peer-connection 1 host =>offer sdp?? 
2.  then in browser uncheck role 'offering'
3.  paste the offer sdp from the server console into the second textarea in a browser 'paste remote description'
4.  then in the first textarea in a browser copy 'copy local description' and paste it into the server console
* Notes. after paste into the server console 'enter' and see if thre are parser errors of some kind
