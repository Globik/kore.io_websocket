FU=`pkg-config --cflags glib-2.0 nice`
a: dtls.c
	gcc -fpic -DHAVE_SCTP -DHAVE_SRTP_2 -c $(FU) dtls.c