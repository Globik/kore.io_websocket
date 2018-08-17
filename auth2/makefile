INC = -I/usr/local/include
LI = -L/home/globik/kore.io_websocket/auth2
#-Wl,-rpath /home/globik/kore.io_websocket/auth2 /home/globik/kore.io_websocket/auth2/auth2.so
m: m.c
	gcc -o m m.c
k: k.c
	gcc $(INC) -o k k.c $(LI) -lauth2
