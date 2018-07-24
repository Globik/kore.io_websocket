# kore.io_websocket
Websocket webRTC signaling protocol based on Kore.io framework written in C

Just another little experiment with multiroom chat app. Based on [kore](https://github.com/jorisvink/kore) framework for writing web APIs in C.

[mediasoup](https://github.com/Globik/kore.io_websocket/tree/master/mediasoup) - a prototype of [mediasoup.js](https://github.com/versatica/mediasoup) WebRTC Video Conferencing written on C++ from within kore web framework.
 Using [uv_callback.c](https://github.com/litesync/uv_callback) as a bidirectional comunication between the mediasoup stuff(version 1.2.8) and the kore .

Some basic samples in websocket_x folders.

[websocket_10](https://github.com/Globik/kore.io_websocket/tree/master/websocket_10) - just another proof of concept.

An implemintation of [janus-gateway](https://github.com/meetecho/janus-gateway) - WebRTC gateway into the kore webframework written in C.
Hardcoded to echotest plugin and WebRTC DataChannel API.

## Some experiments with PostgresQL libpq

[websocket_11](https://github.com/Globik/kore.io_websocket/tree/master/websocket_11) - asynchronous LISTEN interface of libpq in a dedicated
thread. Using select for the time being. 



