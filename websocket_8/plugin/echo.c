#include "plugin.h"

j_plugin *plugin_create(void);
int plugin_init(j_cbs*cb,struct kore_task*t);
void plugin_destroy(void);
struct j_plugin_res *plugin_handle_message(char*transaction);


static j_plugin p_m=J_PLUGIN_INIT(
		.init=plugin_init,
		.destroy=plugin_destroy,
		.handle_message=plugin_handle_message,
		);
j_plugin *plugin_create(void){
printf("[%s:%d] Plugin Created!\n", __FILE__, __LINE__);
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
g_print("[%s:%d] ENTERING plugin_message_free\n",__FILE__, __LINE__);
if(!msg || msg==&exit_message) return;
	g_free(msg->transaction);
	msg->transaction=NULL;
	g_free(msg);
	g_print("[%s:%d] plugin_message_free\n",__FILE__, __LINE__);
}

int plugin_init(j_cbs *cbs,struct kore_task*t){
if(g_atomic_int_get(&stopping)){return -1;}
	if(cbs==NULL){return -1;}
	kore_task_channel_write(t,"DUKA!\0",6);
messages=g_async_queue_new_full((GDestroyNotify)plugin_message_free);
	gw=cbs;
	g_atomic_int_set(&initialized,1);
	GError *error=NULL;
	handler_thread=g_thread_try_new("echotest", plugin_handler,/*NULL*/t,&error);
	if(error !=NULL){
	g_atomic_int_set(&initialized,0);
	printf("got error handler_thread: %d\n",error->code);
	return -1;
	}
	g_print("[%s:%d] Echo Plugin Initialized!\n");
	kore_task_channel_write(t,"INIT!\0",6);
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
struct kore_task*t=(struct kore_task*)data;
if(t==NULL)g_print("kore_task is NULL\n");
	//g_print("mem %p\n",t);
//	kore_task_channel_write(t,"LUKA!\0",6);//doesn't work

	
	while(g_atomic_int_get(&initialized) && !g_atomic_int_get(&stopping)){
	msg=g_async_queue_pop(messages);
		if(msg==&exit_message) break;
		if(msg->transaction==NULL){
		plugin_message_free(msg);
			continue;
		}
		g_print("[%s:%d] We got a transaction: %s\n", __FILE__, __LINE__, msg->transaction);
		int res=gw->push_event(&p_m,"HALI HALO FROM ECHO_PLUGIN!",t);

		kore_msg_send(0,MY_MESSAGE_ID,"GLOBIK", 6);
		continue;
		
	}
	plugin_message_free(msg);
	g_print("[%s:%d] leaving thread from PLUGIN_HANDLER\n", __FILE__, __LINE__);
	return NULL;
}
