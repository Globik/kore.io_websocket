// https://github.com/RekGRpth/web-server 
//postgres.c is there
#include "p.h" // postgres_*
//#include "response.h" // response_code_body
#include "macros.h" // DEBUG, ERROR, FATAL
#include <stdlib.h> // malloc, realloc, calloc, free, getenv, setenv, atoi, size_t

#define BYTEAOID 17
#define INT8OID 20
#define INT2OID 21
#define INT4OID 23
#define TEXTOID 25
#define JSONOID 114

static int postgres_connect(uv_loop_t *loop, postgres_t *postgres);
static void postgres_on_poll(uv_poll_t *handle, int status, int events); // void (*uv_poll_cb)(uv_poll_t* handle, int status, int events)
static void postgres_listen(postgres_t *postgres);
static int postgres_socket(postgres_t *postgres);
static int postgres_reset(postgres_t *postgres);
static void postgres_error_result(postgres_t *postgres, PGresult *result);
//static void postgres_error_code_message_length(postgres_t *postgres, enum http_status code, char *message, int length);
static void postgres_success(postgres_t *postgres, PGresult *result);
//static int postgres_code_body(request_t *request, enum http_status code, char *body, int bodylen);
//static int postgres_info_body(request_t *request, char *info, int infolen, char *body, int bodylen);
//static int postgres_push(postgres_t *postgres);
//static int postgres_pop(postgres_t *postgres);
//static int postgres_connection_error(char *sqlstate);
//static enum http_status postgres_sqlstate_to_code(char *sqlstate);
postgres_t *postgres_init_and_connect(uv_loop_t *loop, char *conninfo) {
   DEBUG("loop=%p, conninfo=%s\n", loop, conninfo);
postgres_t *postgres = (postgres_t *)malloc(sizeof(postgres_t));
if (!postgres) { FATAL("malloc\n"); return NULL; }
postgres->conninfo = conninfo;
   // postgres->request = NULL;//bme
  //  pointer_init(&postgres->server_pointer);//bme
if (postgres_connect(loop, postgres)) { FATAL("postgres_connect\n"); postgres_free(postgres); return NULL; }
return postgres;
}

void postgres_free(postgres_t *postgres) {
//    DEBUG("postgres=%p\n", postgres);
   // pointer_remove(&postgres->server_pointer);//bme
   printf("free conn? %p\n",postgres);
    PQfinish(postgres->conn);
    free(postgres);
}

static int postgres_connect(uv_loop_t *loop, postgres_t *postgres) {
   DEBUG("loop=%p, postgres=%p\n", loop, postgres);
    int error = 0;
    postgres->conn = PQconnectStart(postgres->conninfo); // PGconn *PQconnectStart(const char *conninfo)
    if ((error = !postgres->conn)) { FATAL("PQconnectStart\n"); return error; }
    if ((error = PQstatus(postgres->conn) == CONNECTION_BAD)) { FATAL("PQstatus=CONNECTION_BAD\n"); PQfinish(postgres->conn); return error; } // ConnStatusType PQstatus(const PGconn *conn)
//    PQsetErrorVerbosity(postgres->conn, PQERRORS_VERBOSE); // PGVerbosity PQsetErrorVerbosity(PGconn *conn, PGVerbosity verbosity)
    uv_os_sock_t postgres_sock = PQsocket(postgres->conn);
    if ((error = postgres_sock < 0)) { FATAL("PQsocket\n"); PQfinish(postgres->conn); return error; }
    if ((error = uv_poll_init_socket(loop, &postgres->poll, postgres_sock))) {
 FATAL("uv_poll_init_socket\n"); PQfinish(postgres->conn); return error; } 
 // int uv_poll_init_socket(uv_loop_t* loop, uv_poll_t* handle, uv_os_sock_t socket)
    postgres->poll.data = (void *)postgres;
    if ((error = uv_poll_start(&postgres->poll, UV_WRITABLE, postgres_on_poll))) { FATAL("uv_poll_start\n"); PQfinish(postgres->conn); return error; } // int uv_poll_start(uv_poll_t* handle, int events, uv_poll_cb cb)
    return error;
    }

static void postgres_on_poll(uv_poll_t *handle, int status, int events) { // void (*uv_poll_cb)(uv_poll_t* handle, int status, int events)
 DEBUG("ON_POLL? handle=%p, status=%i, events=%i\n", handle, status, events);
    postgres_t *postgres = (postgres_t *)handle->data;
    if (status) { FATAL("status=%i\n", status); postgres_reset(postgres); return; }
    if (PQsocket(postgres->conn) < 0) { FATAL("PQsocket\n"); postgres_reset(postgres); return; } // int PQsocket(const PGconn *conn)
//    DEBUG("PQstatus(postgres->conn)=%i\n", PQstatus(postgres->conn));
    switch (PQstatus(postgres->conn)) { // ConnStatusType PQstatus(const PGconn *conn)
        case CONNECTION_OK: DEBUG("CONNECTION_OK\n"); break;
        case CONNECTION_BAD: FATAL("PQstatus==CONNECTION_BAD %s", PQerrorMessage(postgres->conn)); postgres_reset(postgres); return; // char *PQerrorMessage(const PGconn *conn)
        default: switch (PQconnectPoll(postgres->conn)) { // PostgresPollingStatusType PQconnectPoll(PGconn *conn)
            case PGRES_POLLING_FAILED: FATAL("PGRES_POLLING_FAILED\n"); postgres_reset(postgres); return;
            case PGRES_POLLING_READING: if (uv_poll_start(&postgres->poll, UV_READABLE, postgres_on_poll)) FATAL("uv_poll_start\n"); return; // int uv_poll_start(uv_poll_t* handle, int events, uv_poll_cb cb)
            case PGRES_POLLING_WRITING: if (uv_poll_start(&postgres->poll, UV_WRITABLE, postgres_on_poll)) FATAL("uv_poll_start\n"); return; // int uv_poll_start(uv_poll_t* handle, int events, uv_poll_cb cb)
            case PGRES_POLLING_OK: postgres_listen(postgres); break;
            default: return;
        }
    }
if (events & UV_READABLE) {
	printf("readable\n");
	int a=PQsendQuery(postgres->conn,"SELECT*FROM cokders");
	printf("auuuuuuuuuuuuuuuuuuuuu %d\n",a);
if (!PQconsumeInput(postgres->conn)) { FATAL("PQconsumeInput:%s", PQerrorMessage(postgres->conn)); postgres_reset(postgres); return; } // int PQconsumeInput(PGconn *conn); char *PQerrorMessage(const PGconn *conn)
if (PQisBusy(postgres->conn)) return; // int PQisBusy(PGconn *conn)
for (PGresult *result;(result = PQgetResult(postgres->conn)); PQclear(result)) switch (PQresultStatus(result)) { 
		// PGresult *PQgetResult(PGconn *conn); void PQclear(PGresult *res); ExecStatusType PQresultStatus(const PGresult *res)
            case PGRES_TUPLES_OK: postgres_success(postgres, result); break;
            case PGRES_FATAL_ERROR: ERROR("PGRES_FATAL_ERROR\n"); postgres_error_result(postgres, result); break;
            default: break;
        }
        
for (PGnotify *notify; (notify = PQnotifies(postgres->conn)); PQfreemem(notify)) { 
	// PGnotify *PQnotifies(PGconn *conn); void PQfreemem(void *ptr)
  DEBUG("Asynchronous notification \"%s\" with payload \"%s\" received from server process with PID %d.\n", notify->relname, notify->extra, notify->be_pid);
}
        
      //  if (postgres_push(postgres)) FATAL("postgres_push\n");//bme
    }
    if (events & UV_WRITABLE) printf("writable\n");switch (PQflush(postgres->conn)) { // int PQflush(PGconn *conn);
	int a=PQsendQuery(postgres->conn,"SELECT*FROM coders");
	printf("auuuuuuuuuuuuuuuuuuuuu %d\n",a);
        case 0: /*DEBUG("No data left to send\n"); */if (uv_poll_start(&postgres->poll, UV_READABLE, postgres_on_poll)) FATAL("uv_poll_start\n"); break; // int uv_poll_start(uv_poll_t* handle, int events, uv_poll_cb cb)
        case 1: DEBUG("More data left to send\n"); break;
        default: FATAL("error sending query\n"); break;
    }
}

static void postgres_listen(postgres_t *postgres) {
    DEBUG("postgres=%p\n", postgres);
    if (!PQisnonblocking(postgres->conn) && PQsetnonblocking(postgres->conn, 1)) FATAL("PQsetnonblocking:%s", PQerrorMessage(postgres->conn)); // int PQisnonblocking(const PGconn *conn); int PQsetnonblocking(PGconn *conn, int arg); char *PQerrorMessage(const PGconn *conn)
    if (!PQsendQuery(postgres->conn, "listen \"webserver\";")) { FATAL("PQsendQuery:%s", PQerrorMessage(postgres->conn)); return; }// int PQsendQuery(PGconn *conn, const char *command); char *PQerrorMessage(const PGconn *conn)
    if (uv_poll_start(&postgres->poll, UV_WRITABLE, postgres_on_poll)) { FATAL("uv_poll_start\n"); return; } // int uv_poll_start(uv_poll_t* handle, int events, uv_poll_cb cb)
}

static int postgres_socket(postgres_t *postgres) {
    DEBUG("postgres=%p\n", postgres);
    int error = 0;
    if ((error = PQsocket(postgres->conn) < 0)) { FATAL("PQsocket\n"); postgres_reset(postgres); return error; } // int PQsocket(const PGconn *conn)
    if ((error = PQstatus(postgres->conn) != CONNECTION_OK)) { FATAL("PQstatus!=CONNECTION_OK\n"); postgres_reset(postgres); return error; } // ConnStatusType PQstatus(const PGconn *conn)
    return error;
}
static int postgres_reset(postgres_t *postgres) {
//    DEBUG("postgres=%p, postgres->request=%p\n", postgres, postgres->request);
    int error = 0;
    if (uv_is_active((uv_handle_t *)&postgres->poll)) if ((error = uv_poll_stop(&postgres->poll))) { FATAL("uv_poll_stop\n"); return error; } // int uv_is_active(const uv_handle_t* handle); int uv_poll_stop(uv_poll_t* poll)
   // if (postgres->request) if (request_push(postgres->request)) FATAL("request_push\n");//bme
   // postgres->request = NULL;//bme
   // pointer_remove(&postgres->server_pointer);//bme
//    PQfinish(postgres->conn);
//    if ((error = postgres_connect(postgres->poll.loop, postgres))) { FATAL("postgres_connect\n"); postgres_reset(postgres); return error; }
    if ((error = !PQresetStart(postgres->conn))) { FATAL("PQresetStart\n"); return error; } // int PQresetStart(PGconn *conn);
    postgres->poll.io_watcher.fd = PQsocket(postgres->conn);
    if ((error = uv_poll_start(&postgres->poll, UV_WRITABLE, postgres_on_poll))) { FATAL("uv_poll_start\n"); return error; } // int uv_poll_start(uv_poll_t* handle, int events, uv_poll_cb cb)
    return error;
}

static void postgres_error_result(postgres_t *postgres, PGresult *result) {
    char *message = PQresultErrorMessage(result); // char *PQresultErrorMessage(const PGresult *res)
    FATAL("\n%s", message);
    if (postgres_socket(postgres)) { FATAL("postgres_socket\n"); return; }
  //  char *sqlstate = PQresultErrorField(result, PG_DIAG_SQLSTATE); //bme
  // char *PQresultErrorField(const PGresult *res, int fieldcode)
   // if (postgres_connection_error(sqlstate)) return;//bme
  //  request_t *request = postgres->request;//bme
//    DEBUG("sqlstate=%s\n", sqlstate);
  //  if (postgres_code_body(request, postgres_sqlstate_to_code(sqlstate), message, strlen(message))) FATAL("postgres_code_body\n");//bme
}
int postgres_cancel(postgres_t *postgres) {
//    DEBUG("postgres=%p\n", postgres);
    int error = 0;
    //postgres->request = NULL;//bme
    if ((error = postgres_socket(postgres))) { FATAL("postgres_socket\n"); return error; }
   // if (!PQisBusy(postgres->conn)) return postgres_push(postgres); //bme
    PGcancel *cancel = PQgetCancel(postgres->conn); // PGcancel *PQgetCancel(PGconn *conn)
    if ((error = !cancel)) { FATAL("PQgetCancel\n"); return error; }
    int errbufsize = 256; char errbuf[errbufsize];
    if ((error = !PQcancel(cancel, errbuf, errbufsize))) FATAL("PQcancel:%s\n", errbuf); // int PQcancel(PGcancel *cancel, char *errbuf, int errbufsize)
    PQfreeCancel(cancel); // void PQfreeCancel(PGcancel *cancel)
//    if ((error = postgres_push(postgres))) { FATAL("postgres_push\n"); return error; }
    return error;
}

static void postgres_success(postgres_t *postgres, PGresult *result) {
   DEBUG("result=%p, postgres=%p\n", result, postgres);
    //request_t *request = postgres->request;
    char *error = NULL;
    if (PQntuples(result) != 1 || PQnfields(result) != 2) error = "1 row and 2 cols expected"; // int PQntuples(const PGresult *res); int PQnfields(const PGresult *res);
    int info = PQfnumber(result, "info");
    if (info == -1) error = "info col expected";
    if (PQftype(result, info) != TEXTOID) error = "info col must be text"; // Oid PQftype(const PGresult *res, int column_number);
    int body = PQfnumber(result, "body");
    if (body == -1) error = "body col expected";
    if (PQftype(result, body) != BYTEAOID) error = "body col must be bytea"; // Oid PQftype(const PGresult *res, int column_number);
    //if (error) { if (request) postgres_error_code_message_length(postgres, HTTP_STATUS_NO_RESPONSE, error, strlen(error)); return; }
   // if (postgres_info_body(request, PQgetvalue(result, 0, info), PQgetlength(result, 0, info), PQgetvalue(result, 0, body), PQgetlength(result, 0, body))) FATAL("postgres_info_body\n");
}

int main(){
	uv_loop_t *loop=uv_default_loop();
	printf("start\n");
	postgres_t *p=postgres_init_and_connect(loop,"dbname=postgres");
	if(p==NULL){printf("erri bye\n");exit(0);return 0;}
	printf("adr of postgres: %p\n",p);
	//int a=PQsendQuery(p->conn,"SELECT*FROM csoders");
	//printf("auuuuuuuuuuuuuuuuuuuuu %d\n",a);
	uv_run(loop,UV_RUN_DEFAULT);
	printf("after loop?\n");
	if(p)postgres_free(p);
	postgres_free(p);
	return 0;
	}
