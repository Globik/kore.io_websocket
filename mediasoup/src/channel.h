#pragma once
#include "soup_server.h"
#include "uv_callback.h"
//struct channel;
//typedef void(*on_ersti_cb)(struct channel*,void*);
//typedef void(*on_channel_send)(struct channel*, char*, on_ersti_cb);//d
struct soup;
struct channel{
ee_t*ee;
void (*request)(struct channel*,char*,struct soup*);
//on_ersti_cb on_ersti;

};
struct channel * channel_new(void);
void ersti_cb(struct channel*,void*);
struct responsi{
struct channel*ch;
void*data;
};
void * on_from_cpp(uv_callback_t*,void*);
uv_callback_t to_cpp;
