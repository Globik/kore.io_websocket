#gcc -o l l.c `pkg-config --cflags --libs glib-2.0`
INCLS=-I/home/globik/janus-gateway -I/home/globik/janus-gateway/transports \
-I/home/globik/janus-gateway/plugins -I/home/globik/janus-gateway/events -I/home/globik/webrtc
OBJ=/home/globik/janus-gateway/*.o /home/globik/janus-gateway/plugins/janus-plugin.o 
DEFS=-DINET -DINET6 -DDEBUG_SCTP -DHAVE_LIBCURL -DHAVE_SCTP -DEBUG -D_GNU_SOURCE -DH_SRTP_2 -DDEBUG_LEVEL=7 
CFLAGS=-g -O2 -std=c99  $(DEFS) -fPIC -Wno-deprecated `pkg-config --cflags openssl nice`
SUKA=-pthread -lgobject-2.0 -lusrsctp -lcurl -lsrtp2 -ldl -ljansson
PAA=/home/globik/janus-gateway
SUKA3= $(PAA)/log.c $(PAA)/apierror.c $(PAA)/config.c $(PAA)/rtp.c $(PAA)/rtcp.c $(PAA)/record.c $(PAA)/utils.c $(PAA)/plugins/plugin.c \
$(PAA)/sdp.c $(PAA)/sdp-utils.c $(PAA)/events.c $(PAA)/ice.c $(PAA)/dtls.c $(PAA)/dtls-bio.c \
$(PAA)/ip-utils.c $(PAA)/turnrest.c $(PAA)/text2pcap.c $(PAA)/sctp.c $(PAA)/auth.c

SU2=log.o apierror.o config.o rtp.o rtcp.o record.o utils.o plugin.o sdp.o sdp-utils.o events.o ice.o dtls.o dtls-bio.o ip-utils.o \
turnrest.o text2pcap.o sctp.o $(PAA)/janus-version.o auth.o b.o

HELPI=helper.o handle_sdp.o dlsym.o

SU3=$(PAA)/janus-version.o events.o ice.o log.o b.o dtls.o

EVENT=-L /usr/local/lib/janus/events -ljanus_sampleevh
h: helper.c
	gcc $(INCLS) $(CFLAGS) -c helper.c handle_sdp.c dlsym.c
b: b.c
	gcc $(INCLS) $(CFLAGS) -rdynamic -o b b.c  $(HELPI) $(OBJ) $(SUKA) `pkg-config --libs glib-2.0 nice openssl` 

s: b.c
	gcc $(INCLS) $(CFLAGS) -c -fpic b.c 

j: j
	gcc $(INCLS) $(CFLAGS) -c -fpic janus_echotest.c 
	
v: v
	gcc -shared -DDEBUG -DLOG_INFO=7 -o libjanus_echtest.so janus_echotest.o -ljansson $(EVENT) \
	`pkg-config --libs glib-2.0  nice openssl` -lsrtp2 -lusrsctp -lm -pthread -ldl -lcrypto -lssl
