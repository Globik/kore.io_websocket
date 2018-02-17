#include "plugin.h"

j_plugin *plugin_create(void);
int plugin_init(j_cbs*cb);
void plugin_destroy(void);
struct j_plugin_res *plugin_handle_message(char*transaction);


static j_plugin p_m=J_PLUGIN_INIT(
		.init=plugin_init,
		.destroy=plugin_destroy,
		.handle_message=plugin_handle_message,
		);
j_plugin *plugin_create(void){
printf("Created!\n");
return &p_m;
}
static volatile gint initialized=0,stopping=0;
static j_cbs *gw=NULL;
static GThread *handler_thread;
static void *plugin_handler(void*data);
typedef struct j_message{
char*transaction;
} j_message;
static GAsyncQueue *messages=NULL;
static j_message exit_message;

static void plugin_message_free(j_message *msg){
g_print("ENTERING plugin_message_free\n");
if(!msg || msg==&exit_message) return;
	g_free(msg->transaction);
	msg->transaction=NULL;
	g_free(msg);
	g_print("plugin_message_free\n");
}

int plugin_init(j_cbs *cbs){
if(g_atomic_int_get(&stopping)){return -1;}
	if(cbs==NULL){return -1;}
messages=g_async_queue_new_full((GDestroyNotify)plugin_message_free);
	gw=cbs;
	g_atomic_int_set(&initialized,1);
	GError *error=NULL;
	handler_thread=g_thread_try_new("echotest",plugin_handler,NULL,&error);
	if(error !=NULL){
	g_atomic_int_set(&initialized,0);
	printf("got error handler_thread: %d\n",error->code);
	return -1;
	}
	g_print("Echo Plugin Initialized!\n");
return 0;
}

void plugin_destroy(void){
if(!g_atomic_int_get(&initialized)) return;
	g_atomic_int_set(&stopping,1);
	g_async_queue_push(messages, &exit_message);
	if(handler_thread !=NULL){
	g_thread_join(handler_thread);
		handler_thread=NULL;
	}
	g_async_queue_unref(messages);
	messages=NULL;
	g_atomic_int_set(&initialized,0);
	g_atomic_int_set(&stopping,0);
	printf("Destroyd\n");
}

struct j_plugin_res *plugin_handle_message(char*transaction){
if(g_atomic_int_get(&stopping) || !g_atomic_int_get(&initialized))
	return j_plugin_res_new(J_PLUGIN_ERROR, g_atomic_int_get(&stopping) ? "shutting down" : "plugin not initialized");
	j_message*msg=g_malloc(sizeof(j_message));
	msg->transaction=transaction;
	g_async_queue_push(messages,msg);
	return j_plugin_res_new(J_PLUGIN_OK_WAIT,"i'm taking my time");
}

static void*plugin_handler(void*data){
j_message *msg=NULL;
	while(g_atomic_int_get(&initialized) && !g_atomic_int_get(&stopping)){
	msg=g_async_queue_pop(messages);
		if(msg==&exit_message) break;
		if(msg->transaction==NULL){
		plugin_message_free(msg);
			continue;
		}
		g_print("got a message: %s\n",msg->transaction);
		//int res=gw->push_event("FUCKER_Fucker_AS_ANSWER FROM ECHO PLUGIN");
		int res=gw->push_event(&p_m,"FUCKER_Fucker_AS_ANSWER_FROM_ECHO_PLUGIN");
		g_print("res of gw->push_event(FUCKER_fucker): %d\n",res);
		continue;
		
		//plugin_message_free(msg); ???
		
	}
	plugin_message_free(msg);
	g_print("leaving thread from PLUGIN_HANDLER\n");
	return NULL;
}