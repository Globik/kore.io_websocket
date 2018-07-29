#include <signal.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
//#include <unistd.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>
#include <kore/tasks.h>

#include "assets.h"

#define REQ_STATE_INIT			0
#define REQ_STATE_QUERY			1
#define REQ_STATE_DB_WAIT		2
#define REQ_STATE_DB_READ		3
#define REQ_STATE_ERROR			4
#define REQ_STATE_DONE			5

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
int konnikov=0;
int init(int);
void foo(int);
int		page(struct http_request *);
int do_loop(struct kore_task*);
void pipe_data_available(struct kore_task *);
//for ranger
static int	request_perform_init(struct http_request *);
static int	request_perform_query(struct http_request *);
static int	request_db_wait(struct http_request *);
static int	request_db_read(struct http_request *);
static int	request_error(struct http_request *);
static int	request_done(struct http_request *);
//for page
static int	request_perform_init2(struct http_request *);
static int	request_perform_query2(struct http_request *);
static int	request_db_wait(struct http_request *);
static int	request_db_read2(struct http_request *);
static int	request_error2(struct http_request *);
static int	request_done2(struct http_request *);

//callback binding pgsql interface

void connection_del(struct connection*);
void connection_new(struct connection*);
void db_state_change(struct kore_pgsql*,void*);
void db_init(struct connection*,struct kore_pgsql*,int);
void db_results(struct kore_pgsql*,struct connection*);


int fuck=0;
static int done=0;
const char*listenchannel="revents";
void kore_worker_configure(void);

int page_ws_connect(struct http_request*);
void websocket_connect(struct connection*);
void websocket_disconnect(struct connection*);
void websocket_message(struct connection*,u_int8_t,void*,size_t);


void handler(void);
void han(int);
void mainloop(PGconn*conn, struct kore_task*);
void exitclean(PGconn*conn);
void handlepgread(PGconn*conn, struct kore_task*);
void initlisten(PGconn*conn);
static void han_sig(int);

void sse_ping(void *, u_int64_t);
int ranger(struct http_request *);
int	dpage(struct http_request *);
int	subscribe(struct http_request *);
void sse_disconnect(struct connection *);
void sse_send(struct connection *, void *, size_t);
void sse_broadcast(struct connection *, void *, size_t);

void brod(struct connection*,void*,size_t);

int	check_header(struct http_request *, const char *, const char *);
struct sse_state {
struct kore_timer*timer;
};


PGconn*conn=NULL;
struct kore_task pipe_task;
//for ranger
struct http_state	mystates[] = {
	{ "REQ_STATE_INIT",	request_perform_init },
	{ "REQ_STATE_QUERY", request_perform_query },
	{ "REQ_STATE_DB_WAIT", request_db_wait },
	{ "REQ_STATE_DB_READ", request_db_read },
	{ "REQ_STATE_ERROR", request_error },
	{ "REQ_STATE_DONE", request_done },
};
//for page
struct http_state	mystates2[] = {
	{ "REQ_STATE_INIT",	request_perform_init2 },
	{ "REQ_STATE_QUERY", request_perform_query2 },
	{ "REQ_STATE_DB_WAIT", request_db_wait },
	{ "REQ_STATE_DB_READ", request_db_read2 },
	{ "REQ_STATE_ERROR", request_error2 },
	{ "REQ_STATE_DONE", request_done2 },
};
//ranger
#define mystates_size		(sizeof(mystates) / sizeof(mystates[0]))
//page
#define mystates_size2		(sizeof(mystates2) / sizeof(mystates2[0]))

struct rstate {
int cnt;
char*name;
struct kore_pgsql sql;
};
struct dura{
int fuck;
struct kore_pgsql*pgsql;
};
void websocket_connect(struct connection*c){
kore_log(LOG_INFO,yellow "ws client connected. %p" rst,(void*)c);
connection_new(c);
}
void websocket_disconnect(struct connection*c){
kore_log(LOG_INFO, yellow "ws client disconnected.%p" rst,(void*)c);	
connection_del(c);
}
void websocket_message(struct connection*c,u_int8_t op,void*data,size_t len){
kore_log(LOG_INFO, yellow "on ws message." rst);

db_init(c,c->hdlr_extra,1);

kore_websocket_send(c,op,data,len);	
}
int page_ws_connect(struct http_request*req){
kore_websocket_handshake(req,"websocket_connect","websocket_message","websocket_disconnect");
return (KORE_RESULT_OK);	
}

int init(int state){
if(state==KORE_MODULE_UNLOAD) return (KORE_RESULT_ERROR);
//if(worker->id !=1) return (KORE_RESULT_OK);
kore_task_create(&pipe_task,do_loop);
kore_task_bind_callback(&pipe_task, pipe_data_available);
kore_task_run(&pipe_task);
// Register our database.
//kore_pgsql_register("db", "host=/tmp dbname=test");
kore_pgsql_register("db","dbname=postgres");
return (KORE_RESULT_OK);
}
void handler(){
kore_log(LOG_INFO, "at exit[worker] handler occured.\n");
done=1;
raise(SIGUSR1);
}
int ba=0;
void han(int a){
printf("han occured.\n");
ba++;
raise(SIGUSR1);
if(ba==3)exit(0);
}
void kore_worker_configure(){
kore_log(LOG_NOTICE, "worker configure\n");
atexit(handler);
}


int page(struct http_request *req)
{
	const char*custom;
	const char*custom2;
	char*body;
	ssize_t ret;
	struct kore_buf*buf;
	u_int8_t data[BUFSIZ];
kore_log(LOG_NOTICE,"%p: page start",(void*)req);
if(req->method !=HTTP_METHOD_POST){
	http_response(req,HTTP_STATUS_METHOD_NOT_ALLOWED,"ono",3);
	return (KORE_RESULT_OK);
	}
//xhr.setRequestHeader('Content-Type','application/json','utf-8');
//xhr.setRequestHeader('X-Requested-With','XMLHttpRequest');
if(http_request_header(req,"X-Requested-With",&custom)){
	kore_log(LOG_INFO,green "it is a: %s" rst,custom);
	}
if(http_request_header(req,"Content-Type",&custom2)){
	kore_log(LOG_INFO,green "it is a: %s" rst,custom2);
	}
	
	buf=kore_buf_alloc(128);
	for(;;){
		ret=http_body_read(req,data,sizeof(data));
		if(ret==-1){
			kore_buf_free(buf);
			kore_log(LOG_INFO,red "error reading body" rst);
			http_response(req,500,"nob",3);
			return (KORE_RESULT_OK);
			}
			if(ret==0)break;
			kore_buf_append(buf,data,ret);
		}
		body=kore_buf_stringify(buf,NULL);
		kore_log(LOG_INFO, yellow "data buffer: %s" rst, body);
		kore_buf_free(buf);

	//http_response(req,200,"ok\n",3);
	//return (KORE_RESULT_OK);
return (http_state_run(mystates2, mystates_size2, req));
}

int request_perform_init(struct http_request *req)
{
kore_log(LOG_INFO, yellow "***perform init: %s ****\n" rst, req->path);
struct rstate	*state;
if (!http_state_exists(req)) {
state = http_state_create(req, sizeof(*state));
state->cnt=20;
kore_pgsql_init(&state->sql);
kore_pgsql_bind_request(&state->sql, req);
} else {
state = http_state_get(req);
}
if (!kore_pgsql_setup(&state->sql, "db", KORE_PGSQL_ASYNC)) {

if (state->sql.state == KORE_PGSQL_STATE_INIT) {
req->fsm_state = REQ_STATE_INIT;
kore_log(LOG_INFO, yellow "kore_pgsql_state_init\n" rst);
return (HTTP_STATE_RETRY);
}
kore_pgsql_logerror(&state->sql);
req->fsm_state = REQ_STATE_ERROR;
} else {
kore_log(LOG_INFO, yellow "The initial setup was complete, go for query.\n" rst);
req->fsm_state = REQ_STATE_QUERY;
}
return (HTTP_STATE_CONTINUE);
}

int request_perform_init2(struct http_request *req)
{
kore_log(LOG_INFO, yellow "***perform init: %s ****\n" rst, req->path);
struct rstate	*state;
if (!http_state_exists(req)) {
state = http_state_create(req, sizeof(*state));
state->cnt=20;
kore_pgsql_init(&state->sql);
kore_pgsql_bind_request(&state->sql, req);
} else {
state = http_state_get(req);
}

if (!kore_pgsql_setup(&state->sql, "db", KORE_PGSQL_ASYNC)) {
if (state->sql.state == KORE_PGSQL_STATE_INIT) {
req->fsm_state = REQ_STATE_INIT;
kore_log(LOG_INFO, yellow "kore_pgsql_state_init\n" rst);
return (HTTP_STATE_RETRY);
}
kore_pgsql_logerror(&state->sql);
req->fsm_state = REQ_STATE_ERROR;
} else {
kore_log(LOG_INFO, yellow "The initial setup was complete, go for query.\n" rst);
req->fsm_state = REQ_STATE_QUERY;
}
return (HTTP_STATE_CONTINUE);
}

// ranger
int request_perform_query(struct http_request *req){
struct rstate	*state = http_state_get(req);
kore_log(LOG_INFO, yellow "request_perform_query(). state->cnt: %d" rst,state->cnt); 
req->fsm_state = REQ_STATE_DB_WAIT;
if (!kore_pgsql_query(&state->sql,"update banners set alt='feodor'")) {
kore_log(LOG_INFO, red "Let the state machine continue immediately since we have an error anyway.\n" rst);
return (HTTP_STATE_CONTINUE);
}
return (HTTP_STATE_RETRY);
}
// page
int request_perform_query2(struct http_request *req){
struct rstate	*state = http_state_get(req);
kore_log(LOG_INFO, yellow "request_perform_query2(). state->cnt: %d" rst,state->cnt); 
req->fsm_state = REQ_STATE_DB_WAIT;
if (!kore_pgsql_query(&state->sql,"update coders set name='Linux'")) {
kore_log(LOG_INFO, red "Let the state machine continue immediately since we have an error anyway." rst);
return (HTTP_STATE_CONTINUE);
}
return (HTTP_STATE_RETRY);
}

int request_db_wait(struct http_request *req){
struct rstate	*state = http_state_get(req);
kore_log(LOG_NOTICE, "request_db_wait(): %d", state->sql.state);
switch (state->sql.state) {
case KORE_PGSQL_STATE_WAIT:
kore_log(LOG_INFO, yellow "http_state_retry" rst);
return (HTTP_STATE_RETRY);
case KORE_PGSQL_STATE_COMPLETE:
kore_log(LOG_INFO, yellow "req_state_done" rst);
req->fsm_state = REQ_STATE_DONE;
break;
case KORE_PGSQL_STATE_ERROR:
kore_log(LOG_INFO, red "state_error" rst);
req->fsm_state = REQ_STATE_ERROR;
kore_pgsql_logerror(&state->sql);
break;
case KORE_PGSQL_STATE_RESULT:
kore_log(LOG_INFO, yellow "req_state_db_read\n" rst);
req->fsm_state = REQ_STATE_DB_READ;
break;
default:
kore_log(LOG_INFO, yellow "kore_pgsql_continue\n" rst);
kore_pgsql_continue(&state->sql);
break;
}
kore_log(LOG_INFO, yellow "returning continue\n" rst);
return (HTTP_STATE_CONTINUE);
}
//ranger
int request_db_read(struct http_request *req){
//char		*name;
//int		i, rows;
struct rstate	*state = http_state_get(req);
kore_log(LOG_INFO, yellow "We have sql data to read!" rst);

/*
	rows = kore_pgsql_ntuples(&state->sql);
	printf("rows: %d\n",rows);
	for (i = 0; i < rows; i++) {
		name = kore_pgsql_getvalue(&state->sql, i, 0);
		kore_log(LOG_NOTICE, "name: '%s'", name);
		suka=name;
		state->name=kore_strdup(name);
	}
	*/ 
//printf("cmd status: %s\n",PQcmdStatus(res));
//state->name="DUMA";
//printf(yellow "Continue processing our query results.\n" rst);
kore_pgsql_continue(&state->sql);
//printf(yellow "Back to our DB waiting state.\n" rst);
req->fsm_state = REQ_STATE_DB_WAIT;
return (HTTP_STATE_CONTINUE);
}

//page:
int request_db_read2(struct http_request *req){

struct rstate	*state = http_state_get(req);
kore_log(LOG_INFO, yellow "We have sql data to read! db_read2. state->cnt: %d" rst,state->cnt);
kore_pgsql_continue(&state->sql);
req->fsm_state = REQ_STATE_DB_WAIT;
return (HTTP_STATE_CONTINUE);
}

//An error occurred. ranger:
int request_error(struct http_request *req)
{
kore_log(LOG_INFO, red "an error occured in db request." rst);
struct rstate	*state = http_state_get(req);
kore_pgsql_cleanup(&state->sql);
http_state_cleanup(req);
http_response(req, 500, NULL, 0);
return (HTTP_STATE_COMPLETE);
}
//page:
int request_error2(struct http_request *req)
{
kore_log(LOG_INFO, red "an error occured in dberror 2" rst);
struct rstate	*state = http_state_get(req);
kore_pgsql_cleanup(&state->sql);
http_state_cleanup(req);
http_response(req, 500, NULL, 0);
return (HTTP_STATE_COMPLETE);
}
//Request was completed successfully. ranger:
int request_done(struct http_request *req){
u_int16_t id;
struct kore_buf*buf;
char*sid;
printf(yellow "***REQUEST_DONE(): %s****" rst, req->path);
struct rstate	*state = http_state_get(req);

http_populate_get(req);
buf = kore_buf_alloc(128);
//Grab it as a string, we shouldn't free the result in sid. 
if (http_argument_get_string(req, "id", &sid)){
kore_log(LOG_INFO,red "***SID!!!*** %s" rst,sid);
kore_buf_appendf(buf, "id as a string: '%s'\n", sid);
}
//Grab it as an actual u_int16_t.
if (http_argument_get_uint16(req, "id", &id))
kore_buf_appendf(buf, "id as an u_int16_t: %d\n", id);
//Now return the result to the client with a 200 status code. 
http_response(req, 200, buf->data, buf->offset);
kore_buf_free(buf);
kore_pgsql_cleanup(&state->sql);
http_state_cleanup(req);
return (HTTP_STATE_COMPLETE);
}

//page:
int request_done2(struct http_request *req){
//u_int16_t id;
//struct kore_buf*buf;
//char*sid;
printf(yellow "***REQUEST_DONE_2(): %s****\n" rst, req->path);
struct rstate	*state = http_state_get(req);

//http_populate_get(req);
//buf = kore_buf_alloc(128);
//Grab it as a string, we shouldn't free the result in sid. 
//if (http_argument_get_string(req, "id", &sid)){
//kore_log(LOG_INFO,red "***SID!!!*** %s" rst,sid);
//kore_buf_appendf(buf, "id as a string: '%s'\n", sid);
//}
//Grab it as an actual u_int16_t.
//if (http_argument_get_uint16(req, "id", &id))
//kore_buf_appendf(buf, "id as an u_int16_t: %d\n", id);
//Now return the result to the client with a 200 status code. 
http_response(req, 200, "pagedone", 8);
//kore_buf_free(buf);
kore_pgsql_cleanup(&state->sql);
http_state_cleanup(req);
return (HTTP_STATE_COMPLETE);
}
void pipe_data_available(struct kore_task *t){
struct connection*c;
size_t len;
u_int8_t buf[BUFSIZ];
if(kore_task_finished(t)){
kore_log(LOG_NOTICE,"Task finished.");
return;
}
len=kore_task_channel_read(t,buf,sizeof(buf));
if(len > sizeof(buf)){printf("len great than buf\n");}
kore_log(LOG_INFO,"leni: %d\n", len);
kore_log(LOG_NOTICE,"Task msg: %s",buf);
//brod(c,buf,len);
char*leaving="event: leave\ndata: miracle\n\n";
//brod(c,leaving,strlen(leaving));
char*leav="event: leave\ndata: ";
size_t leav_len=strlen(leav);
kore_log(LOG_INFO,"leav_len: %d",leav_len);
struct kore_buf*sbuf;
sbuf=kore_buf_alloc(leav_len+len);
char*li="putin";
//kore_buf_appendf(sbuf,"mama mia: %s\n\n",li);
kore_buf_appendf(sbuf,"%s",leav);
kore_buf_appendf(sbuf,"%s\n\n",buf);
kore_log(LOG_NOTICE,"sbuf: %s",sbuf->data);
brod(c,sbuf->data,sbuf->offset);
kore_buf_free(sbuf);

}
int do_loop(struct kore_task*t){
kore_task_channel_write(t,"mama\0",5);
signal(SIGUSR1,foo);
const char*conninfokeys[]={"dbname",NULL};
const char*conninfovalues[]={"postgres",NULL};
conn=PQconnectStartParams(conninfokeys,conninfovalues,0);
ConnStatusType status=PQstatus(conn);
if(status==CONNECTION_BAD){
kore_log(LOG_INFO, red "connection database failed %s" rst, PQerrorMessage(conn));
exitclean(conn);
return (KORE_RESULT_OK);
}else if(status==CONNECTION_STARTED){
kore_log(LOG_INFO, "connection started!");
}else if(status==CONNECTION_MADE){
kore_log(LOG_INFO, green "Connection made." rst);	
}else{kore_log(LOG_INFO, yellow "Connecting..." rst);}
mainloop(conn,t);
PQfinish(conn);
conn=NULL;
kore_log(LOG_INFO, green "***bye!***" rst);
return (KORE_RESULT_OK);
}
void exitclean(PGconn*conn){
printf(yellow "exitclean(conn) occured.\n");

if(done==0){if(conn !=NULL) PQfinish(conn);}
done=1;
conn=NULL;
}
int i=0;
static void han_sig(int n){

i++;
printf("n: %d\n",n);
printf(yellow "han_sig SIGINT occured.\n" rst);
done=1;
if(konnikov==0){printf(green "not connected, return." rst);return;}
if(conn !=NULL)PQfinish(conn);
conn=NULL;
if(i==3)exit(0);
}
void foo(int n){
kore_log(LOG_INFO, yellow "FOO occured." rst);
if(konnikov==0){printf(green "not connected, return." rst);return;}
if(conn !=NULL){
kore_log(LOG_INFO, "conn is not null in foo");done=1;
PQfinish(conn);
conn=NULL;

}else{kore_log(LOG_INFO, "conn is null in foo");done=1;}
konnikov=0;
}
void mainloop(PGconn*conn,struct kore_task*t){
fd_set rfds,wfds;
int retval;
int sock;
int connected=0;
	
int u=PQsetnonblocking(conn,1);
kore_log(LOG_INFO, "non blocking: %d",u);
int ud=PQisnonblocking(conn);
// 1 nonbl, 0 bl
kore_log(LOG_INFO, "is non blocking? :%d",ud);

sock=PQsocket(conn);	
if(sock<0){
kore_log(LOG_INFO,red "postgres socket is gone" rst);
exitclean(conn);
}
int mt=fcntl(sock,F_DUPFD_CLOEXEC,0);
if(mt<0){
kore_log(LOG_INFO, red "failed dupfd_cloexec: %s" rst,strerror(errno));
}				
PostgresPollingStatusType connstatus;

while(!done){
//printf("*WHILE_LOOP*\n");
//usleep(20000);
if(!connected){
connstatus=PQconnectPoll(conn);
switch(connstatus){
case PGRES_POLLING_FAILED:
kore_log(LOG_NOTICE, red "pgconn failed: %s" rst,PQerrorMessage(conn));
done=1;
exitclean(conn);
break;
case PGRES_POLLING_WRITING:
kore_log(LOG_INFO, "PGRES_POLLING_WRITING");
//	FD_SET(sock,&wfds);
break;
case PGRES_POLLING_READING:
kore_log(LOG_INFO, "PGRES_POLLING_READING");
FD_SET(sock,&rfds);
break;
case PGRES_POLLING_OK:
kore_log(LOG_INFO, green "PGRES_POLLING_OK" rst);
connected=1;
konnikov=1;
initlisten(conn);
break;
}
}
//if(connected){printf("CONNECTED=true\n");}
retval=select(sock+1,&rfds,NULL,NULL,NULL);
//printf("after select\n");
switch(retval){
case -1:
//perror("select failed\n");
if(errno==EINTR){
// the fuck it is in a dedicated thread like this, it does not work like in a main thread or process.
kore_log(LOG_INFO, red "EINTR occurred.");
}
kore_log(LOG_INFO, red "done??" rst);
connected=0;
done=1;
break;
case 0:
kore_log(LOG_INFO,"CASE 0\n");
break;
default:
if(!connected){
kore_log(LOG_INFO, "Not connected.\n");
break;
}
if(FD_ISSET(sock,&rfds)){kore_log(LOG_INFO, yellow "DO HANDLE PG READ: retval: %d %d\n" rst,retval,sock);handlepgread(conn,t);}
kore_log(LOG_INFO, "default\n");
}
}
}

void initlisten(PGconn*conn){
kore_log(LOG_INFO, "Entering initlisten()\n");
char*quotedchannel=PQescapeIdentifier(conn,listenchannel,strlen(listenchannel));
char*query;
asprintf(&query,"LISTEN %s",quotedchannel);
int qs=PQsendQuery(conn,"LISTEN revents;LISTEN on_coders"/*query*/);
PQfreemem(quotedchannel);
free(query);
if(!qs){
kore_log(LOG_INFO,red "Failed to send query: %s" rst,PQerrorMessage(conn));
return;
}
}
void handlepgread(PGconn*conn,struct kore_task*t){
kore_log(LOG_INFO, "entering handlepgread(conn)");
PGnotify*notify;
PGresult*res;
if(!PQconsumeInput(conn)){
kore_log(LOG_INFO,red "failed to consume input: %s" rst, PQerrorMessage(conn));
//done=1;
//exitclean(conn);
return;
}
while((res=PQgetResult(conn)) !=NULL){
if(PQresultStatus(res) !=PGRES_COMMAND_OK){
kore_log(LOG_INFO,red "result err: %s" rst, PQerrorMessage(conn));
PQclear(res);
return;
}

kore_log(LOG_INFO, "PQresultStatus: %s",PQresStatus(PQresultStatus(res)));
//printf("returns rows: %d\n",PQntuples(res));
//printf("cols: %d\n",PQnfields(res));
kore_log(LOG_INFO, "cmd status: %s",PQcmdStatus(res));
fuck=1;
PQclear(res);
}

//printf("before notify\n");
while(notify=PQnotifies(conn)){
kore_log(LOG_INFO,yellow "notify of %s received from backend pid %d ,extra: %s" rst, notify->relname, notify->be_pid, notify->extra);
//kore_task_channel_write(t,"DAMA\0",5);
int fs=strlen(notify->extra);
kore_log(LOG_INFO, green "fs len: %d" rst, fs);
kore_task_channel_write(t, notify->extra, fs+1);
PQfreemem(notify);
}
//printf("END\n");
}
//end of async listen - notify
int dpage(struct http_request *req)
{
if (req->method != HTTP_METHOD_GET) {
http_response_header(req, "allow", "get");
http_response(req, 405, NULL, 0);
return (KORE_RESULT_OK);
}

http_response_header(req, "content-type", "text/html");
http_response(req, 200, asset_index_html, asset_len_index_html);
return (KORE_RESULT_OK);
}

int subscribe(struct http_request *req)
{
struct sse_state*state;
char*hello = "event:join\ndata: client\n\n";
// Preventive paranoia. 
if (req->hdlr_extra != NULL) {
kore_log(LOG_ERR, "%p: already subscribed", req->owner);
http_response(req, 500, NULL, 0);
return (KORE_RESULT_OK);
}
//Only allow GET methods.
if (req->method != HTTP_METHOD_GET) {
http_response_header(req, "allow", "get");
http_response(req, 405, NULL, 0);
return (KORE_RESULT_OK);
}
//Only do SSE if the client told us it wanted too.
if (!check_header(req, "accept", "text/event-stream"))
return (KORE_RESULT_OK);
//Do not include content-length in our response.
req->flags |= HTTP_REQUEST_NO_CONTENT_LENGTH;
//Notify existing clients of our new client now.
sse_broadcast(req->owner, hello, strlen(hello));
//Set a disconnection method so we know when this client goes away.
req->owner->disconnect = sse_disconnect;
//We do not expect any more data to arrive.
req->owner->flags |= CONN_READ_BLOCK;
//Allocate a state to be carried by our connection.
state = kore_malloc(sizeof(*state));
req->owner->hdlr_extra = state;
//Now start a timer to send a ping back every 10 second.
state->timer = kore_timer_add(sse_ping, 10000, req->owner, 0);
//Respond that the SSE channel is now open.
kore_log(LOG_NOTICE, "%p: connected for SSE", req->owner);
http_response_header(req, "content-type", "text/event-stream");
http_response(req, 200, NULL, 0);
return (KORE_RESULT_OK);
}
void sse_broadcast(struct connection *src, void *data, size_t len)
{
struct connection	*c;
// Broadcast the message to all other clients.
TAILQ_FOREACH(c, &connections, list) {
if (c == src)
continue;
sse_send(c, data, len);
}
}
void brod(struct connection*c,void*data,size_t len){
	//struct connection	*c;
// Broadcast the message to all other clients.
TAILQ_FOREACH(c, &connections, list) {
kore_log(LOG_INFO, green "broad? %p, leni: %d\n" rst, (void*)c,len);
//if (c == src)continue;
sse_send(c, data, len);
}
}

void sse_send(struct connection *c, void *data, size_t len)
{
struct sse_state*state = c->hdlr_extra;
// Do not send to clients that do not have a state.
if (state == NULL) return;
// Queue outgoing data now.
net_send_queue(c, data, len);
net_send_flush(c);
}

void sse_ping(void *arg, u_int64_t now)
{
struct connection *c = arg;
char *ping = "event:ping\ndata:\n\n";
//Send our ping to the client.
sse_send(c, ping, strlen(ping));
}

void sse_disconnect(struct connection *c)
{
struct sse_state*state = c->hdlr_extra;
char*leaving = "event: leave\ndata: client\n\n";
kore_log(LOG_NOTICE, "%p: disconnecting for SSE", c);
//Tell others we are leaving.
sse_broadcast(c, leaving, strlen(leaving));
//Kill our timer and free/remove the state.
kore_timer_remove(state->timer);
kore_free(state);
//Prevent us to be called again.
c->hdlr_extra = NULL;
c->disconnect = NULL;
}

int check_header(struct http_request *req, const char *name, const char *value)
{
const char*hdr;
if (!http_request_header(req, name, &hdr)) {
http_response(req, 400, NULL, 0);
return (KORE_RESULT_ERROR);
}
if (strcmp(hdr, value)) {
http_response(req, 400, NULL, 0);
return (KORE_RESULT_ERROR);
}
return (KORE_RESULT_OK);
}
int ranger(struct http_request *req)
{
// AJAX get request with some params plus request to database
// yeah, callback hell. Like in nodejs land. But what can I do about it?
//coroutines are also not nice, error prone, even much harder to develope than callback hell, no adecvate libs like in c++, etc...
return (http_state_run(mystates, mystates_size, req));
}

void connection_new(struct connection*c){
struct kore_pgsql*pgsql;
struct dura*g=kore_calloc(1,sizeof(*g));
g->fuck=999;
//c->disconnect=connection_del;
//c->proto=CONN_PROTO_UNKNOWN;
c->state=CONN_STATE_ESTABLISHED;
pgsql=kore_calloc(1,sizeof(*pgsql));
kore_pgsql_init(pgsql);
kore_pgsql_bind_callback(pgsql,db_state_change, c);

//c->hdlr_extra=pgsql;
g->pgsql=pgsql;
c->hdlr_extra=g;
kore_log(LOG_INFO, yellow "new calback db connection: %p" rst,(void*)c);
//printf("fuck: %d\n",c->fuck);
//db_init(c,pgsql);	
}

void db_init(struct connection*c,struct kore_pgsql*pgsql,int a){
kore_log(LOG_INFO,"a: %d",a);

if(!kore_pgsql_setup(pgsql,"db",KORE_PGSQL_ASYNC)){
if(pgsql->state==KORE_PGSQL_STATE_INIT){
kore_log(LOG_INFO,"waiting for available pgsql connection");
return;	
}
kore_log(LOG_INFO, red "err here" rst);
kore_pgsql_logerror(pgsql);
//kore_connection_disconnect(c);
return;	
}
kore_log(LOG_INFO,green "got pgsql connection" rst);
if(!kore_pgsql_query(pgsql,"select * from coders")){
kore_log(LOG_INFO,red "err here2" rst);
kore_pgsql_logerror(pgsql);
//kore_connection_disconnect(c);
return;	
}
kore_log(LOG_INFO,yellow "query fired off!" rst);	
}

void connection_del(struct connection*c){
//kore_connection_disconnect(c);
kore_log(LOG_INFO, yellow "connection db cb disconnecting..: %p" rst,(void*)c);
if(c->hdlr_extra !=NULL)kore_pgsql_cleanup(c->hdlr_extra);
kore_free(c->hdlr_extra);
c->hdlr_extra=NULL;	
//kore_connection_disconnect(c);
}
void db_state_change(struct kore_pgsql*pgsql,void*arg){
struct connection*c=arg;
kore_log(LOG_INFO,"%p state change on pgsql cb %d",arg,pgsql->state);
switch(pgsql->state){
case KORE_PGSQL_STATE_INIT:
kore_log(LOG_INFO, yellow "***cb state db_init***" rst);
db_init(c,pgsql,1);
break;
case KORE_PGSQL_STATE_WAIT:
kore_log(LOG_INFO, yellow "cb state wait" rst);
break;
case KORE_PGSQL_STATE_COMPLETE:
kore_log(LOG_INFO, yellow "cb state complete" rst);
//connection_del(c);
//kore_connection_disconnect(c);
//kore_pgsql_cleanup(c->hdlr_extra);
kore_log(LOG_INFO, yellow "c->hdlr_extra = pgsql, %p = %p" rst,(void*)c->hdlr_extra,(void*)pgsql);
kore_pgsql_cleanup(pgsql);
//kore_connection_disconnect(c);// is a need for a websocket?
// a bug? after ctrl + C =>
//(index):94 WebSocket connection to 'ws://127.0.0.1:8888/connect' failed: 
//One or more reserved bits are on: reserved1 = 1, reserved2 = 1, reserved3 = 0
break;
case KORE_PGSQL_STATE_ERROR:
kore_log(LOG_INFO, red "cb state error" rst);
kore_pgsql_logerror(pgsql);
kore_connection_disconnect(c);
break;
case KORE_PGSQL_STATE_RESULT:
kore_log(LOG_INFO, yellow "cb state result" rst);
db_results(pgsql,c);
break;
default:
kore_log(LOG_INFO, yellow "cb state default" rst);
kore_pgsql_continue(pgsql);
break;
}	
}
void db_results(struct kore_pgsql*pgsql,struct connection*c){
char *name;int i,rows;
rows=kore_pgsql_ntuples(pgsql);
for(i=0;i<rows;i++){
name=kore_pgsql_getvalue(pgsql,i,0);
//net_send_queue(c,name,strlen(name));	
}
kore_log(LOG_INFO,green "result name: %s" rst,name);
kore_websocket_send(c,WEBSOCKET_OP_TEXT,name,strlen(name));
//net_send_flush(c);
//kore_pgsql_cleanup(pgsql);//???
kore_pgsql_continue(pgsql);	
}
