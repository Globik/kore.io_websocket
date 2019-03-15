# Janus Core within kore.c web framework

Trying intergate Janus Core int kore.c web framwork just for fun.

# Kore.c

[https://github.com/jorisvink/kore](https://github.com/jorisvink/kore)

# Janus (v 0.6.1)

[https://github.com/meetecho/janus-gateway](https://github.com/meetecho/janus-gateway)


##	Goal

A little bit of a web server with janus gateway on board. (For 100 users(not so much))

# Success

I applied kore's websocket as a transport for Janus

I applied the kore's build-in message system for events delivery

It just works. It's very intresting. It's Proof of concept. 

Hardly based on a Janus's Videoroom plugin.

I applied Janus admin API to the same websocket instance.

###  kodev run

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
CONFIGURE WORKER! 
[wrk 0]: *** a dedicated thread! Not to store! ***
[wrk 0]: task_thread: #0 starting
[wrk 0]: task_thread#0: woke up
[wrk 0]: task_thread#0: executing 0xb710c960
Janus commit: f7b02e767debae86f1cea9880e3e29162bb79d30
Compiled on:  Wed Mar 13 12:56:14 EDT 2019

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
[WARN] [src/pjanus.c:Janusmain:3889] Couldn't find any address! using 127.0.0.1 as the local IP... (which is NOT going to work out of your machine)
[WARN] Token based authentication disabled
Initializing recorder code
Initializing ICE stuff (Full mode, ICE-TCP candidates disabled, half-trickle, IPv6 support disabled)
Crypto: OpenSSL pre-1.1.0
[WARN] The libsrtp installation does not support AES-GCM profiles
Fingerprint of our certificate: D2:B9:31:8F:DF:24:D8:0E:ED:D2:EF:25:9E:AF:6F:B8:34:AE:53:9C:E6:F3:8F:F2:64:15:FA:E8:7F:53:2D:38
[ERR] [config.c:janus_config_parse:191]   -- Error reading configuration file 'janus.eventhandler.samplekoremsg.jcfg'... error 2 (No such file or directory)
[WARN] [kore_msg.c:janus_sampleevh_init:152] Couldn't find .jcfg configuration file (janus.eventhandler.samplekoremsg), trying .cfg
*** JANUS SampleEventHandler plugin initialized! ***
[WARN] [src/pjanus.c:Janusmain:4297] Event handler plugin 'libjanus_sampleevh.so' has been disabled, skipping...
sessions, sessions
handles, handles
external, external
jsep, jsep
webrtc, webrtc
media, media
plugins, plugins
transports, transports
core, core
Joining SampleEventHandler handler thread
[ERR] [config.c:janus_config_parse:191]   -- Error reading configuration file 'janus.plugin.streaming.jcfg'... error 2 (No such file or directory)
[WARN] Couldn't find .jcfg configuration file (janus.plugin.streaming), trying .cfg
JANUS Streaming plugin initialized!
[ERR] [config.c:janus_config_parse:191]   -- Error reading configuration file 'janus.plugin.textroom.jcfg'... error 2 (No such file or directory)
[WARN] Couldn't find .jcfg configuration file (janus.plugin.textroom), trying .cfg
JANUS TextRoom plugin initialized!
[ERR] [config.c:janus_config_parse:191]   -- Error reading configuration file 'janus.plugin.echotest.jcfg'... error 2 (No such file or directory)
[WARN] Couldn't find .jcfg configuration file (janus.plugin.echotest), trying .cfg
JANUS EchoTest plugin initialized!
[ERR] [config.c:janus_config_parse:191]   -- Error reading configuration file 'janus.plugin.videoroom.jcfg'... error 2 (No such file or directory)
[WARN] Couldn't find .jcfg configuration file (janus.plugin.videoroom), trying .cfg
 Since this a simple plugin, it does the same for all events: so just convert to string... 
JANUS VideoRoom plugin initialized!
[src/janus.c:61]Got message! from 0 (3143 bytes): [
   {
      "emitter": "MyJanusInstance",
      "type": 256,
      "timestamp": 1552670197678183,
      "event": {
         "status": "started",
         "info": {
            "janus": "server_info",
            "name": "Janus WebRTC Server",
            "version": 61,
            "version_string": "0.6.1",
            "author": "Meetecho s.r.l.",
            "commit-hash": "f7b02e767debae86f1cea9880e3e29162bb79d30",
            "compile-time": "Wed Mar 13 12:56:14 EDT 2019",
            "log-to-stdout": true,
            "log-to-file": false,
            "data_channels": true,
            "accepting-new-sessions": true,
            "session-timeout": 60,
            "reclaim-session-timeout": 0,
            "candidates-timeout": 45,
            "server-name": "MyJanusInstance",
            "local-ip": "127.0.0.1",
            "ipv6": false,
            "ice-lite": false,
            "ice-tcp": false,
            "full-trickle": false,
            "rfc-4588": false,
            "static-event-loops": 0,
            "api_secret": false,
            "auth_token": false,
            "event_handlers": true,
            "transports": {},
            "events": {
               "janus.eventhandler.samplekoremsg": {
                  "name": "JANUS SampleEventHandler plugin",
                  "author": "Meetecho s.r.l.",
                  "description": "This is a trivial sample event handler plugin for Janus, which forwards events via HTTP POST.",
                  "version_string": "0.0.1",
                  "version": 1
               }
            },
            "plugins": {
               "janus.plugin.videoroom": {
                  "name": "JANUS VideoRoom plugin",
                  "author": "Meetecho s.r.l.",
                  "description": "This is a plugin implementing a videoconferencing SFU (Selective Forwarding Unit) for Janus, that is an audio/video router.",
                  "version_string": "0.0.9",
                  "version": 9
               },
               "janus.plugin.textroom": {
                  "name": "JANUS TextRoom plugin",
                  "author": "Meetecho s.r.l.",
                  "description": "This is a plugin implementing a text-only room for Janus, using DataChannels.",
                  "version_string": "0.0.2",
                  "version": 2
               },
               "janus.plugin.streaming": {
                  "name": "JANUS Streaming plugin",
                  "author": "Meetecho s.r.l.",
                  "description": "This is a streaming plugin for Janus, allowing WebRTC peers to watch/listen to pre-recorded files or media generated by gstreamer.",
                  "version_string": "0.0.8",
                  "version": 8
               },
               "janus.plugin.echotest": {
                  "name": "JANUS EchoTest plugin",
                  "author": "Meetecho s.r.l.",
                  "description": "This is a trivial EchoTest plugin for Janus, just used to showcase the plugin interface.",
                  "version_string": "0.0.7",
                  "version": 7
               }
            }
         }
      }
   }
]

^C[wrk 0]: kore_worker_teardown

*** TEARDOWN! ***
saka 0
[parent]: server shutting down
[parent]: waiting for workers to drain and shutdown
Ending sessions timeout watchdog...
JANUS VideoRoom plugin destroyed!
JANUS TextRoom plugin destroyed!
JANUS Streaming plugin destroyed!
JANUS EchoTest plugin destroyed!
LEAVING SAMPLE KORE MSG EVENT HANDLER THREAD!***
*** JANUS SampleEventHandler plugin destoyd!!! ***
suka3
kuku**
fg: 0
do hell occured
[wrk 0]: before kore_task_finish
*** after mutex lock ***
Bye!
[wrk 0]: parent gone, shutting down
JANUS TERMINATION HANDLER
[parent]: worker 0 (2988)-> status 0
[parent]: goodbye

```
###  ldd

```
globik@globik-laptop:~/kore.io_websocket/janus$ ldd janus.so
	linux-gate.so.1 =>  (0xb7767000)
	libpjanus.so => /home/globik/kore.io_websocket/janus/libpjanus.so (0xb7682000)
	libglib-2.0.so.0 => /lib/i386-linux-gnu/libglib-2.0.so.0 (0xb7558000)
	libcurl.so.4 => /usr/lib/i386-linux-gnu/libcurl.so.4 (0xb74ef000)
	libnice.so.10 => /usr/local/lib/libnice.so.10 (0xb74a7000)
	libsrtp2.so.1 => /usr/local/lib/libsrtp2.so.1 (0xb7491000)
	libusrsctp.so.1 => /usr/local/lib/libusrsctp.so.1 (0xb73d5000)
	libdl.so.2 => /lib/i386-linux-gnu/libdl.so.2 (0xb73d0000)
	libc.so.6 => /lib/i386-linux-gnu/libc.so.6 (0xb721f000)
	libconfig.so.9 => /usr/lib/i386-linux-gnu/libconfig.so.9 (0xb7213000)
	libpcre.so.3 => /lib/i386-linux-gnu/libpcre.so.3 (0xb71d4000)
	libpthread.so.0 => /lib/i386-linux-gnu/libpthread.so.0 (0xb71b8000)
	libidn.so.11 => /usr/lib/i386-linux-gnu/libidn.so.11 (0xb7185000)
	librtmp.so.0 => /usr/lib/i386-linux-gnu/librtmp.so.0 (0xb716a000)
	libssl.so.1.0.0 => /lib/i386-linux-gnu/libssl.so.1.0.0 (0xb7112000)
	libcrypto.so.1.0.0 => /lib/i386-linux-gnu/libcrypto.so.1.0.0 (0xb6f63000)
	libgssapi_krb5.so.2 => /usr/lib/i386-linux-gnu/libgssapi_krb5.so.2 (0xb6f1d000)
	liblber-2.4.so.2 => /usr/lib/i386-linux-gnu/liblber-2.4.so.2 (0xb6f0e000)
	libldap_r-2.4.so.2 => /usr/lib/i386-linux-gnu/libldap_r-2.4.so.2 (0xb6ebc000)
	libz.so.1 => /lib/i386-linux-gnu/libz.so.1 (0xb6ea2000)
	librt.so.1 => /lib/i386-linux-gnu/librt.so.1 (0xb6e98000)
	libgio-2.0.so.0 => /usr/lib/i386-linux-gnu/libgio-2.0.so.0 (0xb6d16000)
	libgobject-2.0.so.0 => /usr/lib/i386-linux-gnu/libgobject-2.0.so.0 (0xb6cc4000)
	/lib/ld-linux.so.2 (0xb7768000)
	libgnutls.so.26 => /usr/lib/i386-linux-gnu/libgnutls.so.26 (0xb6bfd000)
	libgcrypt.so.11 => /lib/i386-linux-gnu/libgcrypt.so.11 (0xb6b76000)
	libkrb5.so.3 => /usr/lib/i386-linux-gnu/libkrb5.so.3 (0xb6ab6000)
	libk5crypto.so.3 => /usr/lib/i386-linux-gnu/libk5crypto.so.3 (0xb6a86000)
	libcom_err.so.2 => /lib/i386-linux-gnu/libcom_err.so.2 (0xb6a81000)
	libkrb5support.so.0 => /usr/lib/i386-linux-gnu/libkrb5support.so.0 (0xb6a75000)
	libresolv.so.2 => /lib/i386-linux-gnu/libresolv.so.2 (0xb6a5d000)
	libsasl2.so.2 => /usr/lib/i386-linux-gnu/libsasl2.so.2 (0xb6a41000)
	libgssapi.so.3 => /usr/lib/i386-linux-gnu/libgssapi.so.3 (0xb6a05000)
	libgmodule-2.0.so.0 => /usr/lib/i386-linux-gnu/libgmodule-2.0.so.0 (0xb6a00000)
	libselinux.so.1 => /lib/i386-linux-gnu/libselinux.so.1 (0xb69dd000)
	libffi.so.6 => /usr/lib/i386-linux-gnu/libffi.so.6 (0xb69d5000)
	libtasn1.so.6 => /usr/lib/i386-linux-gnu/libtasn1.so.6 (0xb69c0000)
	libp11-kit.so.0 => /usr/lib/i386-linux-gnu/libp11-kit.so.0 (0xb6984000)
	libgpg-error.so.0 => /lib/i386-linux-gnu/libgpg-error.so.0 (0xb697f000)
	libkeyutils.so.1 => /lib/i386-linux-gnu/libkeyutils.so.1 (0xb697b000)
	libheimntlm.so.0 => /usr/lib/i386-linux-gnu/libheimntlm.so.0 (0xb6972000)
	libkrb5.so.26 => /usr/lib/i386-linux-gnu/libkrb5.so.26 (0xb68eb000)
	libasn1.so.8 => /usr/lib/i386-linux-gnu/libasn1.so.8 (0xb6845000)
	libhcrypto.so.4 => /usr/lib/i386-linux-gnu/libhcrypto.so.4 (0xb6810000)
	libroken.so.18 => /usr/lib/i386-linux-gnu/libroken.so.18 (0xb67fa000)
	libwind.so.0 => /usr/lib/i386-linux-gnu/libwind.so.0 (0xb67d1000)
	libheimbase.so.1 => /usr/lib/i386-linux-gnu/libheimbase.so.1 (0xb67c1000)
	libhx509.so.5 => /usr/lib/i386-linux-gnu/libhx509.so.5 (0xb677a000)
	libsqlite3.so.0 => /usr/lib/i386-linux-gnu/libsqlite3.so.0 (0xb66bd000)
	libcrypt.so.1 => /lib/i386-linux-gnu/libcrypt.so.1 (0xb668c000)
	
```
