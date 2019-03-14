# event plugin
INCLS= -I. -I/home/globik/janus-gateway -I/home/globik/janus-gateway/events `pkg-config --cflags glib-2.0`
CFLAGS=-g
j: kore_msg.c
	gcc $(INCLS) $(CFLAGS) -c -fpic kore_msg.c 
	
v: kore_msg.o
	gcc -shared -o libkore_msg.so kore_msg.o `pkg-config --libs glib-2.0` -pthread -ldl
install: libkore_msg.so
	cp libkore_msg.so /usr/local/lib/janus/events/
configs: janus.eventhandler.samplekoremsg.cfg
	cp janus.eventhandler.samplekoremsg.cfg /usr/local/etc/janus/
