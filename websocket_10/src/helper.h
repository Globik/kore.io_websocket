//#include <limits.h>
#include <stdlib.h> //exit
//#include <net/if.h>
//#include <netdb.h>ijiji
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h> //write
//#include <poll.h>
#include <errno.h> //eintr, errno
#include <glib.h>

#include <kore/kore.h>
#include <kore/tasks.h>

#include "debug.h" //janus_print
#include "utils.h" //janus_pidfile_remove
#include "config.h"
#include "janus.h"
#include "version.h"
#include "events.h"
#include "apierror.h"
#include "log.h"
#include "ip-utils.h"

#ifdef __MACH__
#define SHLIB_EXT "0.dylib"
#else
#define SHLIB_EXT ".so"
#endif

#define JANUS_NAME				"Janus WebRTC Gateway"
#define JANUS_AUTHOR			"Meetecho s.r.l."
#define JANUS_VERSION			25
#define JANUS_VERSION_STRING	"0.2.5"
#define JANUS_SERVER_NAME		"MyJanusInstance"

#define DEFAULT_SESSION_TIMEOUT		3000

#define CONFDIR "/home/globik/kore.io_websocket/websocket_10/configs"

#define WEBSOCKET_PAYLOAD_SINGLE	125
#define WEBSOCKET_PAYLOAD_EXTEND_1	126
#define WEBSOCKET_PAYLOAD_EXTEND_2	127
//#define CONFDIR "/usr/local/etc/janus"

struct ex{
int id;
int b;
guint64 sender_id;
};

//#include "log.h"
extern const char* janus_get_api_error(int);
extern  gint stop_signal;
extern  volatile gint stop;
extern gboolean daemonize;
extern int pipefd[2];
gchar *server_name;
//static 
	janus_mutex sessions_mutex;
//static
	GHashTable *old_sessions, *sessions;
//static 
	uint session_timeout;
//static 
	GMainContext *sessions_watchdog_context;
//static
extern	gchar *local_ip;
//static 
	gchar *public_ip;

//static
	GHashTable *plugins;
//static 
	GHashTable *plugins_so;

const char *path;
extern janus_config *config;
//extern 
janus_config_item*item;
gboolean force_bundle;
gboolean force_rtcpmux;


//DIR *dir;
//struct dirent *pluginent;
gchar **disabled_plugins;
//static 
	//char *config_file;
//static 
	//char *configs_folder;
//static janus_callbacks janus_handler_plugin;
struct dirent *pluginent;
char *configs_folder;
DIR *dir;
int janus_log_level;
gboolean janus_log_timestamps;
gboolean janus_log_colors;
int lock_debug;
//static 
	gboolean notify_events;
janus_plugin *fucker;

void janus_handle_signal(int);
void janus_termination_handler(void);
gint janus_is_stopping(void);
gboolean janus_cleanup_session(gpointer);
void janus_session_free(janus_session *);
janus_session *janus_session_find(guint64);
janus_session *janus_session_create(guint64);
gpointer janus_sessions_watchdog(gpointer);
//static 
	gboolean janus_check_sessions(gpointer);
//static 
	void janus_session_schedule_destruction(janus_session *,gboolean, gboolean, gboolean);
gchar *janus_get_local_ip(void);
gchar *janus_get_public_ip(void);
void janus_set_public_ip(const char *ip);
void janus_plugin_close(gpointer, gpointer, gpointer);
janus_plugin *janus_plugin_find(const gchar *);
void janus_session_notify_event(janus_session *, json_t *);
void janus_plugin_notify_event(janus_plugin *, janus_plugin_session *, json_t *);
void janus_pluginso_close(gpointer, gpointer, gpointer);
void janus_plugin_end_session(janus_plugin_session *);
void janus_plugin_close_pc(janus_plugin_session *);
void janus_plugin_relay_data(janus_plugin_session *, char *, int);
void janus_plugin_relay_rtp(janus_plugin_session *, int, char *, int);
void janus_plugin_relay_rtcp(janus_plugin_session *, int, char *, int);
int janus_plugin_push_event(janus_plugin_session *,janus_plugin *,const char *,json_t *, json_t *); 
json_t *janus_plugin_handle_sdp(janus_plugin_session *plugin_session, janus_plugin *plugin, const char *sdp_type, const char *sdp,gboolean);
void plug_fucker(GHashTable *,janus_plugin*);
void load_plugin(const char*);

//static 
	janus_callbacks janus_handler_plugin; //=
/*
	{
		.push_event = janus_plugin_push_event,
		.relay_rtp = janus_plugin_relay_rtp,
		.relay_rtcp = janus_plugin_relay_rtcp,
		.relay_data = janus_plugin_relay_data,
		.close_pc = janus_plugin_close_pc,
		.end_session = janus_plugin_end_session,
		.events_is_enabled = janus_events_is_enabled,
		.notify_event = janus_plugin_notify_event,
	}; 
	*/
gchar*select_local_ip(void);
void conf_max_nack_queue(void);
void conf_force_bundle_or_and_rtcp_mux(void);
void conf_no_media_timer(void);
const char*conf_cert_pem(void);
const char*conf_cert_key(void);
void conf_dtls_mtu(void);

	char*stun_server;
	char*turn_server;
	const char *nat_1_1_mapping;
void test_private_address(void);
void conf_nice_debug(void);

uint16_t stun_port;
uint16_t turn_port;
char *turn_type;
char*turn_user;
char*turn_pwd;
char *turn_rest_api;
char*turn_rest_api_key;
#ifdef HAVE_LIBCURL
char *turn_rest_api_method;
#endif
uint16_t rtp_min_port;
uint16_t rtp_max_port;
gboolean ice_lite;
gboolean ice_tcp;
gboolean ipv6;

void conf_turn(void);
void conf_session_timeout(void);
void conf_interface(void);
void conf_ice_ignore_list(void);
void conf_ice_enforce_list(void);
void stop_signal_hook(void);
gboolean use_stdout;
void puzomerka(gboolean);

void set_conf_file(char*);
void fuck_up(struct kore_task*);

GHashTable *transports;
GHashTable *transports_so;
GHashTable *eventhandlers;
GHashTable *eventhandlers_so;

char *api_secret, *admin_api_secret;
void incoming_message(janus_ice_handle*,janus_session *,json_t*,guint64,struct connection*);
void do_trickle(janus_ice_handle*,janus_session*,json_t*,guint64,struct connection*);








