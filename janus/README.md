# Janus WebRTC gateway innerside of kore.c webframework

```

[parent]: running on http://127.0.0.1:8888
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
[wrk 0]: *** a dedicated thread! Not to store! ***
[wrk 0]: task_thread: #0 starting
[wrk 0]: task_thread#0: woke up
[wrk 0]: task_thread#0: executing 0xb69d88a0
Janus commit: f7b02e767debae86f1cea9880e3e29162bb79d30
Compiled on:  Tue Feb 19 13:34:25 EST 2019

Using CONFDIR
Failed to load /usr/local/etc/janus/janus.jcfg, trying the INI instead...
[ERR] [config.c:janus_config_parse:191]   -- Error reading configuration file 'janus.jcfg'... error 2 (No such file or directory)
---------------------------------------------------
  Starting Meetecho Janus (WebRTC Server) v0.6.1
---------------------------------------------------

Checking command line arguments...
Debug/log level is 4
Debug/log timestamps are disabled
Debug/log colors are enabled
Adding 'vmnet' to the ICE ignore list...
[WARN] Couldn't find any address! using 127.0.0.1 as the local IP... (which is NOT going to work out of your machine)
Using 127.0.0.1 as local IP...
[WARN] Token based authentication disabled
Initializing recorder code
Initializing ICE stuff (Full mode, ICE-TCP candidates disabled, half-trickle, IPv6 support disabled)
Crypto: OpenSSL pre-1.1.0
[WARN] The libsrtp installation does not support AES-GCM profiles
Fingerprint of our certificate: D2:B9:31:8F:DF:24:D8:0E:ED:D2:EF:25:9E:AF:6F:B8:34:AE:53:9C:E6:F3:8F:F2:64:15:FA:E8:7F:53:2D:38
Event handler plugins folder: /usr/local/lib/janus/events
Setting event handlers statistics period to 5 seconds
[WARN] Event handler plugin 'libjanus_sampleevh.so' has been disabled, skipping...
sessions, sessions
handles, handles
external, external
jsep, jsep
webrtc, webrtc
media, media
plugins, plugins
transports, transports
core, core
Plugins folder: /usr/local/lib/janus/plugins
Loading plugin 'libjanus_streaming.so'...
[ERR] [config.c:janus_config_parse:191]   -- Error reading configuration file 'janus.plugin.streaming.jcfg'... error 2 (No such file or directory)
[WARN] Couldn't find .jcfg configuration file (janus.plugin.streaming), trying .cfg
JANUS Streaming plugin initialized!
Loading plugin 'libjanus_textroom.so'...
[ERR] [config.c:janus_config_parse:191]   -- Error reading configuration file 'janus.plugin.textroom.jcfg'... error 2 (No such file or directory)
[WARN] Couldn't find .jcfg configuration file (janus.plugin.textroom), trying .cfg
JANUS TextRoom plugin initialized!
Loading plugin 'libjanus_echotest.so'...
[ERR] [config.c:janus_config_parse:191]   -- Error reading configuration file 'janus.plugin.echotest.jcfg'... error 2 (No such file or directory)
[WARN] Couldn't find .jcfg configuration file (janus.plugin.echotest), trying .cfg
JANUS EchoTest plugin initialized!
Loading plugin 'libjanus_videoroom.so'...
[ERR] [config.c:janus_config_parse:191]   -- Error reading configuration file 'janus.plugin.videoroom.jcfg'... error 2 (No such file or directory)
Sessions watchdog started
[WARN] Couldn't find .jcfg configuration file (janus.plugin.videoroom), trying .cfg
JANUS VideoRoom plugin initialized!
^CStopping server, please wait...
In a hurry? I'm trying to free resources cleanly, here!
[parent]: server shutting down
[parent]: waiting for workers to drain and shutdown
Ending sessions timeout watchdog...
Sessions watchdog stopped
Closing transport plugins:
Destroying sessions...
Freeing crypto resources...
De-initializing SCTP...
Closing plugins:
JANUS VideoRoom plugin destroyed!
JANUS TextRoom plugin destroyed!
JANUS Streaming plugin destroyed!
JANUS EchoTest plugin destroyed!
Closing event handlers:
Bye!
[parent]: worker 0 (18015)-> status 0
[parent]: goodbye

```


#  Echotest DataChannel in browser console



```


message {"janus": "success", "transaction": "session_create", "data": {"id": 5220996969483049}}
(index):41 message {"janus": "success", "session_id": 5220996969483049, "transaction": "attach_plugin", "data": {"id": 3064473378753647}}
(index):149 ON NEGOTIATION NEEDED!
pc.onnegotiationneeded @ (index):149
(index):163 SEND OFFER to answer.html
(anonymous) @ (index):163
Promise.then (async)
set_local_desc @ (index):162
Promise.then (async)
gotLocalStream @ (index):152
publish @ (index):120
onclick @ (index):25
(index):146 ice gathering:  gathering
(index):134 ON ICE CANDIDATE!
pc.onicecandidate @ (index):134
(index):41 message {"janus": "event", "session_id": 5220996969483049, "transaction": "configure", "sender": 3064473378753647, "plugindata": {"plugin": "janus.plugin.echotest", "data": {"echotest": "event", "result": "ok"}}, "jsep": {"type": "answer", "sdp": "v=0\r\no=- 6067414302225579263 2 IN IP4 10.34.33.192\r\ns=-\r\nt=0 0\r\na=group:BUNDLE data\r\na=msid-semantic: WMS janus\r\nm=application 9 DTLS/SCTP 5000\r\nc=IN IP4 10.34.33.192\r\na=sendrecv\r\na=sctpmap:5000 webrtc-datachannel 16\r\na=mid:data\r\na=ice-ufrag:1utY\r\na=ice-pwd:8ISs3ZW704PfrjxMQ5O9kU\r\na=ice-options:trickle\r\na=fingerprint:sha-256 D2:B9:31:8F:DF:24:D8:0E:ED:D2:EF:25:9E:AF:6F:B8:34:AE:53:9C:E6:F3:8F:F2:64:15:FA:E8:7F:53:2D:38\r\na=setup:active\r\na=candidate:1 1 udp 2013266431 10.34.33.192 40471 typ host\r\na=end-of-candidates\r\n"}}
(index):183 HANDLE_ANSWER: 
 {type: "answer", sdp: "v=0
↵o=- 6067414302225579263 2 IN IP4 10.34.33.192…0.34.33.192 40471 typ host
↵a=end-of-candidates
↵"}
handle_answer @ (index):183
sock.onmessage @ (index):63
(index):41 message {"janus": "ack", "session_id": 5220996969483049, "transaction": "configure", "hint": "I'm taking my time!"}
(index):41 message {"janus": "ack", "session_id": 5220996969483049, "transaction": "candidate"}
(index):145 ice connection state:  checking
(index):146 ice gathering:  complete
(index):187 settled remote sdp
(anonymous) @ (index):187
Promise.then (async)
handle_answer @ (index):186
sock.onmessage @ (index):63
(index):145 ice connection state:  connected
(index):145 ice connection state:  completed
(index):41 message {"janus": "webrtcup", "session_id": 5220996969483049, "sender": 3064473378753647}

``` 
