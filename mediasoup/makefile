INC = -I ./soup/channel -I ./soup
#INC_CPPJSON=-I /home/globik/.local/share/Trash/files/deps.2/jsoncpp/jsoncpp/include
# /home/globik/kore.io_websocket/uv3/libuv_callback.so
# /home/globik/alikon/node_modules/mediasoup/worker/deps/jsoncpp/jsoncpp/include/json/json.h
INC_CPPJSON=-I /home/globik/alikon/node_modules/mediasoup/worker/deps/jsoncpp/jsoncpp/include
CBINC = -I /home/globik/kore.io_websocket/uv3
I=$(INC) $(INC_CPPJSON) $(CBINC) `pkg-config --cflags libuv`
CBLIB = -Wl,-rpath /home/globik/kore.io_websocket/uv3 -L /home/globik/kore.io_websocket/uv3 -luv_callback
LIBJSON = -L /home/globik/alikon/node_modules/mediasoup/worker/out/Release -ljsoncpp $(CBLIB)

LSOUP = -Wl,-rpath /home/globik/kore.io_websocket/mediasoup -L /home/globik/kore.io_websocket/mediasoup -lsoup
SRC_PATH = ./soup/
SRC_K = deplibuv.cpp Loop.cpp Room.cpp  UnixStreamSocket.cpp Notifier.cpp Request.cpp Logger.cpp SignalsHandler.cpp
SRC = $(SRC_PATH)*.cpp

LIB_SHAR = `pkg-config --libs libuv` -lcrypto -lssl


OB = deplibuv.o Loop.o Room.o  UnixStreamSocket.o Notifier.o Request.o Logger.o SignalsHandler.o

L = `pkg-config --libs libuv` $(LIBJSON)

LC = `pkg-config --libs libuv` $(CBLIB)

a: $(SRC_PATH)Notifier.cpp
	g++ -std=c++11 $(I) -c $(SRC_PATH)Notifier.cpp
b: $(SRC_PATH)Request.cpp
	g++ -std=c++11 $(I) -c $(SRC_PATH)Request.cpp
c: $(SRC_PATH)Room.cpp
	g++ -std=c++11 $(I) -c $(SRC_PATH)Room.cpp
d: $(SRC_PATH)Loop.cpp
	g++ -fpic -std=c++11 $(I) -c $(SRC_PATH)Loop.cpp
e: $(SRC_PATH)SignalsHandler.cpp
	g++  -fpic-std=c++11 $(I) -c $(SRC_PATH)SignalsHandler.cpp
f: $(SRC_PATH)deplibuv.cpp
	g++ -fpic -std=c++11 $(I) -c $(SRC_PATH)deplibuv.cpp
g: $(SRC_PATH)UnixStreamSocket.cpp
	g++ -fpic -std=c++11 $(I) -c $(SRC_PATH)UnixStreamSocket.cpp
utils: $(SRC_PATH)Crypto.cpp
	g++ -fpic -std=c++11 $(I) -c $(SRC_PATH)Crypto.cpp
# h
i: $(SRC_PATH)Logger.cpp
	g++ -fpic -std=c++11 $(I) -c $(SRC_PATH)Logger.cpp
#k: main.cpp
#	g++ -std=c++11 $(I) -o main main.cpp $(OB) $(L)
l: ./soup/*.cpp
	g++ -fpic -std=c++11 $(I) -c ./soup/*.cpp
m: $(SRC_PATH)Settings.cpp
	g++  -fpic -std=c++11 $(I) -c $(SRC_PATH)Settings.cpp
me: me
	g++ -shared -o libsoup.so *.o $(LIBJSON) 
#$(LIB_SHAR)
#n: m.c
#	gcc $(INC) $(CBINC) -o m m.c $(LC) $(LSOUP)
clean: *.o
	rm *.o
