#ifndef _h_toom_h
#define _h_room_h

struct room{
char* name;
uint32_t roomId;
struct channel* ch;
void(* befree)(struct room*);
//(void* event)();
};

struct room* room_new(uint32_t, struct channel*);//internal, data, channel
void room_free(struct room*);
#endif
