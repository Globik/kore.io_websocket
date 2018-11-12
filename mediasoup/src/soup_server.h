#ifndef _H_SOUP_SERVER
#define _H_SOUP_SERVER
#pragma once
#include <ee.h>
#include "channel.h"
//#include <jansson.h>
#include "globikCommon.h"
//#include "room.h"
#include <stdio.h>

struct soup;
//json_t;

struct server{
	ee_t*ee;
	//char*name;
	struct channel*ch;
	void (*emit)(struct server *, const char*, void*);
	void (*close)(struct server*);
	int (*create_room)(struct server*, struct soup*, const char*);
	void (*destroy)(struct server*);
};

struct soup{
struct server*conn;
u_int8_t state;
uint32_t id;
uint32_t in_id;
char*error;
char*name;
void *arg;
void (*cb)(struct soup *, void*);
char*result;	
};
struct out_data{char*str;};

struct room_holder{
int a;	
}holder;

struct server *server_new(void);
void soup_init(struct soup*, struct server*);
void soup_bind_callback(struct soup*, void (*cb)(struct soup*,void*), void*);
void soup_continue(struct soup*);
int make_room(struct soup*, const char*);
json_t *load_json_str(const char*);

uint32_t random_u32(void);
uint32_t rand32(uint32_t,uint32_t);




#define SOUP_STATE_INIT 1
#define SOUP_STATE_WAIT 2
#define SOUP_STATE_RESULT 3
#define SOUP_STATE_ERROR 4
#define SOUP_STATE_DONE 5
#define SOUP_STATE_COMPLETE 6
#endif
