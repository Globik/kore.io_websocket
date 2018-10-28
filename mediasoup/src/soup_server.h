#pragma once
#include <ee.h>
#include "channel.h"
#include <stdio.h>

//struct server;
struct soup;

//typedef void(*on_emit)(struct server*, const char*, void*);
//typedef void(*on_close)(struct server*);

//typedef int(*on_funny)(void*);
//typedef int(*on_create_room)(struct server*, int);

//void ersti_cb(struct channel*,char*);

struct server{
	ee_t*ee;
	char*name;
	struct channel*ch;
	//on_emit emit;
	void (*emit)(struct server *, const char*, void*);
	//on_close close;
	void (*close)(struct server*);
	int (*create_room)(struct server*,int, struct soup*);
	void (*destroy)(struct server*);
	//int state;
	//void*arg;
	//void (*cb)(struct server*,void*);
};

struct out_data{
const char*str;	
};

struct soup{
struct server*conn;
int state;
char*name;
void *arg;
void (*cb)(struct soup *, void*);	
};

struct server *server_new(void);
void soup_init(struct soup*, struct server*);
void soup_bind_callback(struct soup*, void (*cb)(struct soup*,void*), void*);
int make_room(struct soup*, char*);
