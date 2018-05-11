#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include "ref_counter.h"

struct  media;struct room;
typedef void(*proto_notify)(struct media*, char*);
typedef int(*proto_create_room)(struct media*, char*);
typedef void(*proto_on_new_room)(struct media*, char*);

typedef struct room*(*proto_get_room)(struct media*,gint);
typedef void(*proto_close_room)(struct media*,gint);

struct media{
	int a;
	proto_notify notify;
	proto_create_room create_room;
	proto_on_new_room on_new_room;
	proto_get_room get_room;
	proto_close_room close_room;
	GHashTable *rooms;
	j_refcnt ref;
	};

struct room{
int a;
gint b;
j_refcnt ref;
};
struct media*media_new(proto_notify, proto_on_new_room);
void notify(struct media*, char*);
void on_new_Room(struct media*, char*);
struct room*room_new(void);

