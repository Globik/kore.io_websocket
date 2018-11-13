INCL = -I./ -I /home/globik/ee.c/deps/list
l: l.c
	gcc -o l l.c
l1: l1.c
	gcc $(INCL) -c l1.c
main: main.c
	gcc $(INCL) -o main main.c l1.o -lee
m: m.c
	gcc $(INCL) -o m m.c -lee
