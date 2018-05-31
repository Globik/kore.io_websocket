# /home/globik/libasync/bin/lib/libasync.a
# /home/globik/libasync/include/async
IL = -I ./seq
DEF = -DUM

IUD = -I /home/globik/libasync/include
LUD = -L /home/globik/libasync/bin/lib -lasync
a: um.c
	gcc $(DEF) $(IL) -o um um.c
u: u.c
	gcc -o u u.c
ud: ud.c
	gcc $(IUD) -o ud ud.c $(LUD)