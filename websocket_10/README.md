Kore example websocket server with libjansson

Trying to implement glib mainloop with thread, dlsym for echo plugin. Full cycle. From plugin to websocket channel.

Yeah, today I got it! 

Run:
```
	# kodev run
```

Test:
```
	Open a browser that does websockets, surf to https://127.0.0.1:8888
	or whatever configured IP you have in the config.

	Hit the connect button to open a websocket session, open a second
	tab and surf to the same address and hit the connection button there
	as well. This should cause the number of messages sent/recv to keep
	incrementing as each message is broadcast to the other connection.
```
