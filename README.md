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

[websocket_12](https://github.com/Globik/kore.io_websocket/tree/master/websocket_12) 

[websocket_13](https://github.com/Globik/kore.io_websocket/tree/master/websocket_13) - built-in LISTEN / NOTIFY interface.

With some modifications in source code of pgsql.c Placed PGresult in a while loop. 
This is useful for handling ok's command or results at once after performing multiple queries in a row.

[auth](https://github.com/Globik/kore.io_websocket/tree/master/auth) - 

trying to implement authentication and authorization based on cookies and postgresql storage. 

[template](https://github.com/Globik/kore.io_websocket/tree/master/template) - 

Simple hardcoded html template rendering based on  kore_buf_appendf().

I'm lazy about real template engine. One need to study its lexic etc. No chance for native language to produce some custom microtemplate.

Have you seen ever a template engine where you can recursivly something to render? I'v not seen.

I'm prefer principe: one html page - one template. More flexibility than one head(one footer, one layout for many pages).

It's no go for example one head for many pages. This way there is no chance for custom social unique metatags and css and js files.

Also I don't like an idea to bundle many js or css files to big one. It's nonsense.

With no layout support I can for every page provide some unique css style and unique js script. So page would be downloading  very quickly.

Besides in the future this practice is a right thing with http2 standard.

It's OK with layout support for a blog engine. 

But not the case for an application with many dashboardes, statistics, admin panels, WebRTC possibilities etc.

Hardcoded into the memory? Does no matter. With any file watcher utility I can easily compile the kore and restart it at time of developing stage.








### Other excellent applications based on kore.c webframework:

[https://github.com/stan1y/servo](https://github.com/stan1y/servo) - a session engine and storage for static websites. 
[https://github.com/byronmejia/Pingers](https://github.com/byronmejia/Pingers) - a solution to the Tanda Work-Samples challenge.
[https://github.com/jorisvink/kore-blog](https://github.com/jorisvink/kore-blog) - a tiny blog platform. 




