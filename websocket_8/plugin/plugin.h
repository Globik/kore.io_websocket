#ifndef _PL
#define _PL
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#include <kore/kore.h>
#include <kore/tasks.h>

#define MY_MESSAGE_ID 	100

#define J_PLUGIN_INIT(...){ \
.init=NULL, \
.destroy=NULL, \
.handle_message=NULL, \
## __VA_ARGS__ }

typedef struct j_cbs j_cbs;
typedef struct j_plugin j_plugin;
typedef struct j_plugin_res j_plugin_res;
struct j_plugin{
	int (* const init)(j_cbs *cbs,struct kore_task*t);
	void (* const destroy)(void);
	struct j_plugin_res * (* const handle_message)(char*transaction);
};
struct j_cbs{
struct kore_task*ta;
int (* const push_event)(j_plugin*plugin,const char*transaction,struct kore_task*t);
};

typedef j_plugin* create_p(void);

typedef enum j_plugin_res_type{
	J_PLUGIN_ERROR = -1,
	J_PLUGIN_OK,
	J_PLUGIN_OK_WAIT,
} j_plugin_res_type;

struct j_plugin_res{
j_plugin_res_type type;
	const char*text;
	};
j_plugin_res *j_plugin_res_new(j_plugin_res_type type,const char*text);
void j_plugin_res_destroy(j_plugin_res *result);
#endif
