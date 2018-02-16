
#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
void kore_websocket_broadcast_room(struct connection *, u_int8_t, const void *,size_t, int);