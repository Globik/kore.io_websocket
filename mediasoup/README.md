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
[wrk 0]: A message came: mama
here class init
rc to cpp init: 0
rc from_cpp init: 0
rc stop_w init: 0
version:  1.19.3-dev
Parse all RTP capabilities.
[wrk 0]: rc to_cpp fire 0
Set us as Channel's listener.
unixstreamsocket::setlistener()
loop::mfuck
loop::mfuck
starting libuv loop
Loop was allocated?
HERE AND HERE ON_TO_CPP occured: {"mama":"papa"}
loop::mfuck
unixstreamsocket::useronunixstreamread() {"mama":"papa"}
loop::mfuck
Here inner parse
{
	"mama" : "papa"
}
request 1
here must be within request::request
{
	"mama" : "papa"
}
unixstreamsocket::sendlog() occured.
[id:unset] Channel::UnixStreamSocket::UserOnUnixStreamRead() | discarding wrong Channel request
^Csignal INT received, exiting
[parent]: server shutting down
loop::close() occured
[parent]: waiting for workers to drain and shutdown
CLOSE SIGNALSHANDLER DESTROY
rc fire from_cpp: 0
What the f in destractor in unixstreamsocket?
[wrk 0]: ON_FROM_CPP data came. exit

EXIT!!!
libuv loop ended

loop destructer occured loooop destructure
[wrk 0]: Destoy func.
Loop was destroyd?
[wrk 0]: A message came: mama
[wrk 0]: Bye. *******

[wrk 0]: SUCCESS: And exit with success status.
[parent]: worker 0 (6762)-> status 0
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