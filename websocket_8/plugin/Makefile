#plugin
INCLS= -I. `pkg-config --cflags glib-2.0`
CFLAGS=-g
j: echo.c
	gcc $(INCLS) $(CFLAGS) -c -fpic echo.c 
	
v: echo.o
	gcc -shared -o libecho.so echo.o `pkg-config --libs glib-2.0` -pthread -ldl
