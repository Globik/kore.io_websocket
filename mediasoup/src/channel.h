#pragma once
#include "soup_server.h"
struct channel;
typedef void(*on_ersti_cb)(struct channel*,void*);
typedef void(*on_channel_send)(struct channel*, char*, on_ersti_cb);

struct channel{
ee_t*ee;
on_channel_send request;
on_ersti_cb on_ersti;
};
struct channel * channel_new(void);
void ersti_cb(struct channel*,void*);
struct responsi{
struct channel*ch;
void*data;
};
