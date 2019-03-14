#include <limits.h>
#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
#include <glib.h>
extern gint stop_signal;
extern volatile gint stop;

void kore_websocket_broadcast_room(struct connection *, u_int8_t, const void *,size_t, int);
void kore_websocket_broadcast_room_char(const char*, u_int8_t, const void *, size_t, int);
void j_handle_signal(int);
gpointer j_sess_watchdog(gpointer);
