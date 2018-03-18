# ar -cvq libdima.a  ILibWebRTC.o
OBJ=/home/globik/janus-gateway/*.o /home/globik/janus-gateway/plugins/janus-plugin.o
LI=-L/home/globik/boringssl/build/crypto -Wl,-rpath /home/globik/boringssl/build/crypto -lcrypto -L/home/globik/boringssl/build/ssl -Wl,-rpath /home/globik/boringssl/build/ssl -lssl
PL=-lusrsctp -rdynamic
CFLAGS=-g -rdynamic  -DHAVE_SCTP -DHAVE_SRTP_2  \
-fPIC -Isrc -Isrc/includes -I/usr/local/include -DKORE_NO_TLS -DKORE_USE_TASKS \
-I/home/globik/janus-gateway -I/usr/include/glib-2.0 -I/usr/lib/i386-linux-gnu/glib-2.0/include  \
-I/usr/local/include/nice -I/usr/local/ssl/include 
MIR=-L/usr/local/ssl/lib -Wl,-rpath /usr/local/ssl/lib -lcrypto -lssl
LDFLAGS=-L/usr/local/ssl/lib -Wl,-rpath=/usr/local/ssl/lib -lssl \
-L/usr/local/ssl/lib -Wl,-rpath=/usr/local/ssl/lib -lcrypto \
-ljansson -lcurl -lusrsctp -lsrtp2 -lz -ldl -lglib-2.0 -lnice \
-L/home/globik/kore.io_websocket/websocket_10 -Wl,-rpath /home/globik/kore.io_websocket/websocket_10 -ljanus -lcurl -ldl


c: c
	gcc -shared -o libjanus.so $(OBJ) $(MIR)
b: b
	gcc -c $(CFLAGS) src/websocket_10.c src/dlsym.c src/handle_sdp.c src/helper.c src/incoming.c src/trickle.c
f: f
	gcc -shared -rdynamic -o websocket_10.so *.o $(LDFLAGS) -lcrypto