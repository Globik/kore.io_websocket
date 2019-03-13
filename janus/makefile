P = /home/globik/janus-gateway/
PI =$(P)plugins/janus-plugin.o
OB = $(P)janus-ice.o  $(P)janus-events.o $(P)janus-dtls.o $(P)janus-dtls-bio.o \
 $(P)janus-ip-utils.o $(P)janus-log.o $(P)janus-record.o $(P)janus-rtcp.o \
 $(P)janus-rtp.o $(P)janus-sctp.o $(P)janus-sdp.o $(P)janus-sdp-utils.o \
 $(P)janus-text2pcap.o $(P)janus-turnrest.o $(P)janus-utils.o $(P)janus-version.o \
 $(P)janus-apierror.o $(P)janus-auth.o $(P)janus-config.o $(P)janus-cmdline.o $(PI)
MIR = -L/usr/local/ssl/lib -Wl,-rpath /usr/local/ssl/lib -lcrypto -lssl
a: a
	gcc -shared -rdynamic -fpic -o libpjanus.so $(OB) -lm -lconfig
#janus-text2pcap.o            
#    janus-turnrest.o       
#    janus-utils.o       janus-version.o
