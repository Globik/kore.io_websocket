# Prototype!

Trying to  mimic(in C code) the [mediasoup](https://github.com/versatica/mediasoup/) basis. 
Based on version 1.2.8

Using the [uv_callback](https://github.com/litesync/uv_callback) library as a bidirectional communication between 
the mediasoup(C++) and kore framework(C)

WebRTC Video Conferencing
[https://mediasoup.org](https://mediasoup.org)

## so far so good

### kodev run

```

[parent]: running on http://127.0.0.1:8888
ignoring "tls_dhparam" on line 6
ignoring "certfile" on line 16
ignoring "certkey" on line 17
[parent]: kore is starting up
[parent]: tasks built-in enabled
[wrk 0]: worker 0 started (cpu#0)
[wrk 0]: worker configure
[wrk 0]: A task created
Entering deplibuv::classinit()
uv_callback_t &to_cpp init: 0
uv_callback_t &from_cpp init: 0
Libuv version:  1.19.3-dev
Entering Room::ClassInit().
[wrk 0]: uv_callback_t &to_cpp fire 0
[wrk 0]: A message came: mama
Channel's listener starting.
Entering UnixStreamSocket::SetListener(listener)
A dummy method: loop::mfuck()
Hello libuv's loop!
Loop is allocated successfully
uv_callback_t UnixStreamSocket::on_to_cpp occured!: {"id":3444444333,"method":"worker.createRoom","internal":{"roomId":35,"sister":"sister_1"},"data":{"a":1}}
Entering UnixStreamSocket::UserOnUnixStreamRead() {"id":3444444333,"method":"worker.createRoom","internal":{"roomId":35,"sister":"sister_1"},"data":{"a":1}}
After json parsing.
{
	"data" : 
	{
		"a" : 1
	},
	"id" : 3444444333,
	"internal" : 
	{
		"roomId" : 35,
		"sister" : "sister_1"
	},
	"method" : "worker.createRoom"
}
Creating ::Request.
Entering Request::Request(channel, json)
{
	"data" : 
	{
		"a" : 1
	},
	"id" : 3444444333,
	"internal" : 
	{
		"roomId" : 35,
		"sister" : "sister_1"
	},
	"method" : "worker.createRoom"
}
Request is not nullptr
Entering Loop::OnChannelRequest(channel, request)
'worker.createRoom' request
Getting a room from a request.
No room found.
The room must be created.
Entering Room::Room(listener, notifier, roomId, json
Room created roomId:35]
Entering Request::Accept(json)
Entering UnixStreamSocket::Send(Json)
{
	"accepted" : true,
	"data" : {},
	"id" : 3444444333
}
uv_callback_t &from_cpp fire: 0
Deleting the Request.
Look ma, ~Request() destructor!
[wrk 0]: ON_FROM_CPP data came. HALLO from cpp!!!

^C
 Signal INT received, exiting.
Loop::Close() entered.
[parent]: server shutting down
Closing signalsHandler.
[parent]: waiting for workers to drain and shutdown
Entering Room::Destroy().
Notifier::Emit(uint32_t targetId, const std::string& event, Json::Value& data) occured.
Entering UnixStreamSocket::Send(Json)
{
	"data" : 
	{
		"class" : "Room"
	},
	"event" : "close",
	"targetId" : 35
}
uv_callback_t &from_cpp fire: 0
Entering OnRoomClosed(room)
Look ma, ~Room() destructor!
uv_callback_t &from_cpp fire: 0
Look ma, ~UnixStreamSocket() destructor!
[wrk 0]: ON_FROM_CPP data came. HALLO from cpp!!!

[wrk 0]: ON_FROM_CPP data came. exit

EXIT!!!
Good bye, libuv's loop!
The loop should be ending now!
Look ma, ~Loop() destructor.
[wrk 0]: Destoy func.
Look ma, loop is destroyd.
[wrk 0]: Bye. *******

[wrk 0]: A message came: mama
[wrk 0]: SUCCESS: And exit with success status.
[parent]: worker 0 (6772)-> status 0
[parent]: goodbye

```

## ldd mediasoup.so

```
linux-gate.so.1 =>  (0xb76f7000)
	libuv.so.1 => /usr/local/lib/libuv.so.1 (0xb76c8000)
	libuv_callback.so => /home/globik/kore.io_websocket/uv3/libuv_callback.so (0xb76c4000)
	libsoup.so => /home/globik/kore.io_websocket/mediasoup/libsoup.so (0xb7623000)
	libc.so.6 => /lib/i386-linux-gnu/libc.so.6 (0xb7455000)
	librt.so.1 => /lib/i386-linux-gnu/librt.so.1 (0xb744b000)
	libpthread.so.0 => /lib/i386-linux-gnu/libpthread.so.0 (0xb742f000)
	libdl.so.2 => /lib/i386-linux-gnu/libdl.so.2 (0xb742a000)
	libstdc++.so.6 => /usr/lib/i386-linux-gnu/libstdc++.so.6 (0xb7342000)
	libm.so.6 => /lib/i386-linux-gnu/libm.so.6 (0xb72fc000)
	libgcc_s.so.1 => /lib/i386-linux-gnu/libgcc_s.so.1 (0xb72de000)
	/lib/ld-linux.so.2 (0xb76f8000)


```