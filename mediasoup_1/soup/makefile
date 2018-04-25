# catch  gyp  jsoncpp  libsrtp  libuv  netstring  openssl
#libjsoncpp.a    libopenssl.a  libuv.a           obj.target
#libnetstring.a  libsrtp.a     mediasoup-worker

D = -DMS_LITTLE_ENDIAN
BASE_INC = /home/globik/.local/share/Trash/files/deps.2
BASE_LIB_DIR = /home/globik/alikon/node_modules/mediasoup/worker/out/Release
INC = -I ./include
INC_JSONCPP=-I $(BASE_INC)/jsoncpp/jsoncpp/include
INC_SRTP = -I $(BASE_INC)/libsrtp/srtp/include
INC_OSSL = -I $(BASE_INC)/openssl/include

INC_UV_CALLBACK = -I /home/globik/kore.io_websocket/uv3

UV_CALLBACK_LIB = -Wl,-rpath /home/globik/kore.io_websocket/uv3 -L /home/globik/kore.io_websocket/uv3 -luv_callback
LIB_JSONCPP = -L /home/globik/alikon/node_modules/mediasoup/worker/out/Release -ljsoncpp -lsrtp
LIB_S = -L $(BASE_LIB_DIR) -ljsoncpp -lsrtp
LIB_SHAR = `pkg-config --libs libuv` -lcrypto -lssl $(LIB_S) $(UV_CALLBACK_LIB)
LIB_SOUP = -Wl,-rpath /home/globik/kore.io_websocket/mediasoup_1 -L /home/globik/kore.io_websocket/mediasoup_1 -lsoup

LIBI = $(UV_CALLBACK_LIB) $(LIB_SOUP) `pkg-config --libs libuv`

RTP_DIR = ./src/RTC/RtpDictionaries
RTCP_DIR = ./src/RTC/RTCP
BIT_DIR = ./src/RTC/RemoteBitrateEstimator
RTC_DIR = ./src/RTC
HAN_DIR = ./src/handles
UTI_DIR  = ./src/Utils

I = $(D) $(INC) $(INC_JSONCPP) $(INC_SRTP) $(INC_UV_CALLBACK) $(INC_OSSL) `pkg-config --cflags libuv`

luv: ./src/DepLibUV.cpp
	g++ -std=c++11 $(I) -c ./src/DepLibUV.cpp
loop: ./src/Loop.cpp
	g++ -std=c++11 $(I) -c ./src/Loop.cpp
uss: ./src/Channel/UnixStreamSocket.cpp
	g++ -std=c++11 $(I) -c ./src/Channel/UnixStreamSocket.cpp
noti: ./src/Channel/Notifier.cpp
	g++ -std=c++11 $(I) -c ./src/Channel/Notifier.cpp
req: ./src/Channel/Request.cpp
	g++ -std=c++11 $(I) -c ./src/Channel/Request.cpp
chan: ./src/Channel/*.cpp
	g++ -std=c++11 $(I) -c ./src/Channel/*.cpp
endly: ./src/DepLibSRTP.cpp
	g++ -std=c++11 $(I) -c ./src/DepLibSRTP.cpp ./src/DepLibUV.cpp ./src/DepOpenSSL.cpp ./src/Logger.cpp ./src/Loop.cpp ./src/Settings.cpp
log: ./src/Logger.cpp
	g++ -std=c++11 $(I) -c ./src/Logger.cpp
ssl: ./src/DepOpenSSL.cpp
	g++ -std=c++11 $(I) -c ./src/DepOpenSSL.cpp
tcp: ./src/RTC/TcpServer.cpp
	g++ -std=c++11 $(I) -c ./src/RTC/TcpServer.cpp
srtp: ./src/DepLibSRTP.cpp
	g++ -std=c++11 $(I) -c ./src/DepLibSRTP.cpp
set: ./src/Settings.cpp
	g++ -std=c++11 $(I) -c ./src/Settings.cpp
dtls: ./src/RTC/DtlsTransport.cpp
	g++ -std=c++11 $(I) -c ./src/RTC/DtlsTransport.cpp
rtp: $(RTP_DIR)/*.cpp
	g++ -std=c++11 $(I) -c $(RTP_DIR)/*.cpp
srtpsess: ./src/RTC/SrtpSession.cpp
	g++ -std=c++11 $(I) -c ./src/RTC/SrtpSession.cpp
room: ./src/RTC/Room.cpp
	g++ -std=c++11 $(I) -c ./src/RTC/Room.cpp
rtcp: $(RTCP_DIR)/*.cpp
	g++ -std=c++11 $(I) -c $(RTCP_DIR)/*.cpp
bit: $(BIT_DIR)/*.cpp
	g++ -std=c++11 $(I) -c $(BIT_DIR)/*.cpp
udp: $(RTC_DIR)/UdpSocket.cpp
	g++ -std=c++11 $(I) -c $(RTC_DIR)/UdpSocket.cpp
rtc: $(RTC_DIR)/*.cpp
	g++ -std=c++11 $(I) -c $(RTC_DIR)/*.cpp
han: $(HAN_DIR)/*.cpp
	g++ -std=c++11 $(I) -c $(HAN_DIR)/*.cpp
uti: $(UTI_DIR)/*.cpp
	g++ -std=c++11 $(I) -c $(UTI_DIR)/*.cpp
mama: ./src/mama.cpp
	g++ -std=c++11 $(I) -o m ./src/mama.cpp ./*.o `pkg-config --libs libuv` -lssl -lcrypto -ldl -pthread $(LIBI)
me: me
	g++ -shared -o libsoup.so *.o $(LIB_SHAR)
main: ./src/main.c
	gcc $(I) -o main ./src/main.c `pkg-config --libs libuv` -pthread $(LIBI)
# rtp rtcp bit rtc han uti chan endly
 