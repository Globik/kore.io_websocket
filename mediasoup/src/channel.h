
#ifndef _H_SOUP_CHANNEL
#define _H_SOUP_CHANNEL

#pragma once
#include "soup_server.h"
#include "uv_callback.h"
#include <jansson.h>



struct soup;
struct channel{
ee_t*ee;
void (*request)(struct channel*, struct soup*, const char*, json_t*);// options, internal
void (*close)(struct channel*);
void (*invoke_for_dummy)(struct channel*, json_t*);
};
struct channel * channel_new(void);
void ersti_cb(struct channel*,void*);
struct responsi{
struct channel*ch;
void*data;
};
void * on_from_cpp(uv_callback_t*,void*);
uv_callback_t to_cpp;
#endif
