Kore example websocket server with libjansson

Trying to implement glib mainloop with thread, dlsym for echo plugin. Full cycle. From plugin to websocket channel.

Yeah, today I got it! 
Proof of concept.
Today is a great day. I'm lucky to get to becoming friends among kore.c - as a transport layer for Janus webrtc gateway - and Janus itself.
I got it. Russia, Chelyabinsk-city, 19.03.2018.

One more time I'm saying: a standalone webrtc gateway server is good, but a gateway library is even better.
Theoretically Janus's code could one to add to any good web framework. Today I've proofed it.
No technical education from my side, no expiriencing in programming languages. I just did it and here you are!

## kore web framework

[kore](https://github.com/jorisvink/kore)

## janus webrtc gateway server

[janus](https://github.com/meetecho/janus-gateway)

Just compiled it as libjanus.so. And linked to the kore api. Based(hardcoded) on a libjanus_echotest.so plugin.

Run:
```
	# kodev run
```

Test:
```
	Open a browser that does websockets, surf to https://127.0.0.1:8888
	or whatever configured IP you have in the config.

	Hit the connect button to open a websocket session.
	
	WebRTC DataChannel API in action. Based on hardcoded plugin libjanus_echotest.so
```
