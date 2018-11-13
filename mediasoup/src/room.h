#ifndef _h_toom_h
#define _h_room_h

struct room{
char* name;
uint32_t roomId;
struct channel* ch;
void(* close)(struct room*);
};

struct room* room_new(uint32_t, struct channel*);//internal, data, channel

#endif
