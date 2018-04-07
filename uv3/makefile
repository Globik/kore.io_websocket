LUV = -luv_callback `pkg-config --libs libuv`
INC = -I ./src -I .
a: libuv.cpp
	gcc -fpic `pkg-config --cflags libuv` -c uv_callback.c
b: b
	gcc -shared  -o libuv_callback.so uv_callback.o `pkg-config --libs libuv`
c: libuv.cpp
	g++  -fpic -std=c++11 $(INC) `pkg-config --cflags libuv` -c libuv.cpp
d: libuv.o
	g++ -shared -o libfuk.so libuv.o 
	

e: s.c
	gcc -o s s.c -Wl,-rpath /home/globik/kore-mediasoup/training/luvi/l -L/home/globik/kore-mediasoup/training/luvi/l -lfuk $(LUV)