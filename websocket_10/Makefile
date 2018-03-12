# ar -cvq libdima.a  ILibWebRTC.o
OBJ=/home/globik/janus-gateway/*.o /home/globik/janus-gateway/plugins/janus-plugin.o
LI=-L/home/globik/boringssl/build/crypto -Wl,-rpath /home/globik/boringssl/build/crypto -lcrypto -L/home/globik/boringssl/build/ssl -Wl,-rpath /home/globik/boringssl/build/ssl -lssl
PL=-lusrsctp -rdynamic
b: b
	 ar -cvq libjanus.a  $(OBJ)
c: c
	gcc -shared -o libjanus.so $(OBJ)