INC = -I/usr/include/postgresql -I/usr/local/include/lwan
# /lwan-build-config.h
DEF = -D__GNUC__ -D_POSIX_C_SOURCE=199309L
LIB = `pkg-config --libs lwan`
a: p.c
	gcc $(INC) -o p p.c -lpq $(LIB)
