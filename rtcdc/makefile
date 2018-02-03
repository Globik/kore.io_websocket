#gcc -o l l.c `pkg-config --cflags --libs glib-2.0`
INCLS=-I/home/globik/librtcdc
#LUAP=-L/home/globik/uap-c
CFLAGS=-g -O2   -D_GNU_SOURCE -std=c99  -DINET -DINET6 -DDEBUG_SCTP -fPIC -Wno-deprecated `pkg-config --cflags openssl nice`
d: d.c
	gcc $(INCLS) $(CFLAGS) -o d d.c -L/home/globik/librtcdc/src -lfuck -lusrsctp -lglib-2.0 `pkg-config --libs nice openssl`