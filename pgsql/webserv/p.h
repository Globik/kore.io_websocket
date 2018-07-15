#ifndef _POSTGRES_H
#define _POSTGRES_H

typedef struct postgres_s postgres_t;

//#include "server.h" // server_t
//#include "request.h" // request_t
#include <postgresql/libpq-fe.h> // PQ*, PG*
#include <uv.h> // uv_*

struct postgres_s {
    uv_poll_t poll;
    char *conninfo;
    PGconn *conn;
   // request_t *request;
   // pointer_t server_pointer;
};

postgres_t *postgres_init_and_connect(uv_loop_t *loop, char *conninfo);
void postgres_free(postgres_t *postgres);
int postgres_cancel(postgres_t *postgres);
//int postgres_process(server_t *server);

#endif // _POSTGRES_H
