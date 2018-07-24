/*
 * Copyright (c) 2014-2018 Joris Vink <joris@coders.se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This example demonstrates on how to use state machines and
 * asynchronous pgsql queries. For a synchronous query example
 * see the pgsql-sync/ example under the examples/ directory.
 *
 * While this example might seem overly complex for a simple pgsql
 * query, there is a reason behind its complexity:
 *	Asynchronous pgsql queries mean that Kore will not block while
 *	executing the queries, giving a worker time to continue handling
 *	other events such as I/O or other http requests.
 *
 * The state machine framework present in Kore makes it trivial
 * to get going into dropping from your page handler into the right
 * state that you are currently in.
 */

#if !defined(KORE_NO_HTTP)

#include <signal.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>
#include <kore/tasks.h>

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
int init(int);
void foo(int);
int		page(struct http_request *);
int do_loop(struct kore_task*);
void pipe_data_available(struct kore_task *);

static int	request_perform_init(struct http_request *);
static int	request_perform_query(struct http_request *);
static int	request_db_wait(struct http_request *);
static int	request_db_read(struct http_request *);
static int	request_error(struct http_request *);
static int	request_done(struct http_request *);
//char*suka;
int fuck=0;
static int done;//=0;
const char*listenchannel="revents";
void kore_worker_configure(void);
//void kore_parent_configure(void);
void handler(void);
void han(int);
void mainloop(PGconn*conn);
void exitclean(PGconn*conn);
void handlepgread(PGconn*conn);
void initlisten(PGconn*conn);
static void han_sig(int);
PGconn*conn=NULL;
struct kore_task pipe_task;
struct http_state	mystates[] = {
	{ "REQ_STATE_INIT",		request_perform_init },
	{ "REQ_STATE_QUERY",		request_perform_query },
	{ "REQ_STATE_DB_WAIT",		request_db_wait },
	{ "REQ_STATE_DB_READ",		request_db_read },
	{ "REQ_STATE_ERROR",		request_error },
	{ "REQ_STATE_DONE",		request_done },
};

#define mystates_size		(sizeof(mystates) / sizeof(mystates[0]))

struct rstate {
	int			cnt;
	char*name;
	struct kore_pgsql	sql;
};
int
init(int state)
{
if(state==KORE_MODULE_UNLOAD) return (KORE_RESULT_ERROR);
	//if(worker->id !=1) return (KORE_RESULT_OK);
	printf("after state.\n");

	kore_task_create(&pipe_task,do_loop);
	kore_task_bind_callback(&pipe_task, pipe_data_available);
	kore_task_run(&pipe_task);
	/* Register our database. */
	//kore_pgsql_register("db", "host=/tmp dbname=test");
	kore_pgsql_register("db","dbname=postgres");
	done=0;
	return (KORE_RESULT_OK);
}
void handler(){
	printf("at exit[worker] handler occured.\n");
	done=1;
	raise(SIGUSR1);
	if(conn !=NULL){printf("conn is not null\n");PQfinish(conn);}else{printf("conn is null\n");}
	conn=NULL;
	usleep(100000);
	//exit(0);
	}
	int ba=0;
	void han(int a){
		printf("han occured.\n");
		ba++;
		raise(SIGUSR1);
		if(ba==3)exit(0);
		}
void kore_worker_configure(){
	printf("worker configure\n");
	//signal(SIGINT,han);
	atexit(handler);
	}
void kore_parent_configure(int a,char**j){
	//atexit(handler);
	}
/* Page handler entry point (see config) */
int
page(struct http_request *req)
{
	kore_log(LOG_NOTICE,"%p: page start",(void*)req);
	return (http_state_run(mystates,mystates_size,req));
}

/* Initialize our PGSQL data structure and prepare for an async query. */
int
request_perform_init(struct http_request *req)
{
printf(yellow "perform init\n" rst);
struct rstate	*state;
/* Setup our state context (if not yet set). */
if (!http_state_exists(req)) {
state = http_state_create(req, sizeof(*state));
/*
* Initialize the kore_pgsql data structure and bind it
* to this request so we can be put to sleep / woken up
* by the pgsql layer when required.
*/
kore_pgsql_init(&state->sql);
kore_pgsql_bind_request(&state->sql, req);
} else {
state = http_state_get(req);
}

/*
* Setup the query to be asynchronous in nature, aka just fire it
* off and return back to us.
*/
if (!kore_pgsql_setup(&state->sql, "db", KORE_PGSQL_ASYNC)) {
/*
* If the state was still in INIT we need to go to sleep and
* wait until the pgsql layer wakes us up again when there
* an available connection to the database.
*/
if (state->sql.state == KORE_PGSQL_STATE_INIT) {
req->fsm_state = REQ_STATE_INIT;
printf(yellow "kore_pgsql_state_init\n" rst);
return (HTTP_STATE_RETRY);
}
kore_pgsql_logerror(&state->sql);
req->fsm_state = REQ_STATE_ERROR;
	} else {
printf(yellow "The initial setup was complete, go for query.\n" rst);
req->fsm_state = REQ_STATE_QUERY;
	}

	return (HTTP_STATE_CONTINUE);
}

/* After setting everything up we will execute our async query. */
int
request_perform_query(struct http_request *req)
{
struct rstate	*state = http_state_get(req);
printf(yellow "We want to move to read result after this.\n" rst); 
req->fsm_state = REQ_STATE_DB_WAIT;
/* Fire off the query. */
//if (!kore_pgsql_query(&state->sql,"SELECT * FROM coders, pg_sleep(5)")) {
//if (!kore_pgsql_query(&state->sql,"SELECT * FROM coders")) {
if (!kore_pgsql_query(&state->sql,"update banners set alt='feodor'")) {
printf(red "Let the state machine continue immediately since we have an error anyway.\n" rst);
return (HTTP_STATE_CONTINUE);
}
printf(yellow "Resume state machine later when the query results start coming in.\n" rst);
return (HTTP_STATE_RETRY);
}

/*
 * After firing off the query, we returned HTTP_STATE_RETRY (see above).
 * When request_db_wait() finally is called by Kore we will have results
 * from pgsql so we'll process them.
 */
int
request_db_wait(struct http_request *req)
{
	struct rstate	*state = http_state_get(req);

	kore_log(LOG_NOTICE, "request_db_wait(): %d", state->sql.state);

	/*
	 * When we get here, our asynchronous pgsql query has
	 * given us something, check the state to figure out what.
	 */
	switch (state->sql.state) {
	case KORE_PGSQL_STATE_WAIT:
	printf(yellow "http_state_retry\n" rst);
		return (HTTP_STATE_RETRY);
	case KORE_PGSQL_STATE_COMPLETE:
	printf(yellow "req_state_done\n" rst);
		req->fsm_state = REQ_STATE_DONE;
		break;
	case KORE_PGSQL_STATE_ERROR:
	printf(red "state_error\n" rst);
		req->fsm_state = REQ_STATE_ERROR;
		kore_pgsql_logerror(&state->sql);
		break;
	case KORE_PGSQL_STATE_RESULT:
	printf(yellow "req_state_db_read\n" rst);
		req->fsm_state = REQ_STATE_DB_READ;
		break;
	default:
		/* This MUST be present in order to advance the pgsql state */
		printf(yellow "kore_pgsql_continue\n" rst);
		kore_pgsql_continue(&state->sql);
		break;
	}
	printf(yellow "returning continue\n" rst);

	return (HTTP_STATE_CONTINUE);
}

/*
 * Called when there's an actual result to be gotten. After we handle the
 * entire result, we'll drop back into REQ_STATE_DB_WAIT (above) in order
 * to continue until the pgsql API returns KORE_PGSQL_STATE_COMPLETE.
 */
int
request_db_read(struct http_request *req)
{
	char		*name;
	int		i, rows;
	struct rstate	*state = http_state_get(req);

printf(yellow "We have sql data to read!\n" rst);

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
//suka="duma_1";
state->name="DUMA";
printf(yellow "Continue processing our query results.\n" rst);
kore_pgsql_continue(&state->sql);
printf(yellow "Back to our DB waiting state.\n" rst);
req->fsm_state = REQ_STATE_DB_WAIT;
return (HTTP_STATE_CONTINUE);
}

/* An error occurred. */
int
request_error(struct http_request *req)
{
printf(red "an error occured\n" rst);
struct rstate	*state = http_state_get(req);
kore_pgsql_cleanup(&state->sql);
http_state_cleanup(req);
http_response(req, 500, NULL, 0);
return (HTTP_STATE_COMPLETE);
}

/* Request was completed successfully. */
int
request_done(struct http_request *req)
{
printf(yellow "request done()\n" rst);
struct rstate	*state = http_state_get(req);
printf("DATA: %d\n",state->cnt);
printf(green "DATA NAME: %s\n" rst, state->name);
printf("before cleanup state ->sql\n");
kore_pgsql_cleanup(&state->sql);
printf("before cleanup state req\n");
http_state_cleanup(req);
printf(red "before response HANDLER EXTRA.\n");
http_response(req, 200, NULL,0);
//kore_free(state->name);
//state->name=NULL;
printf("is complete?\n");
return (HTTP_STATE_COMPLETE);
}

#endif /* !KORE_NO_HTTP */
void pipe_data_available(struct kore_task *t){
	size_t len;
	u_int8_t buf[BUFSIZ];
	
if(kore_task_finished(t)){
kore_log(LOG_NOTICE,"Task finished.");
//kore_msg_send(KORE_MSG_PARENT,KORE_MSG_SHUTDOWN,"1",1);
//if(done==1)exit(0);
return;
}

	len=kore_task_channel_read(t,buf,sizeof(buf));
	if(len > sizeof(buf)){printf("len great than buf\n");}
	kore_log(LOG_NOTICE,"Task msg: %s",buf);
	
}
int do_loop(struct kore_task*t){
kore_task_channel_write(t,"mama\0",5);
signal(SIGUSR1,foo);
//signal(SIGTERM,han_sig);	
const char*conninfokeys[]={"dbname",NULL};
const char*conninfovalues[]={"postgres",NULL};
//PGconn*conn=NULL;
conn=PQconnectStartParams(conninfokeys,conninfovalues,0);
ConnStatusType status=PQstatus(conn);
if(status==CONNECTION_BAD){
fprintf(stderr, red "connection database failed %s\n" rst, PQerrorMessage(conn));
exitclean(conn);
}else if(status==CONNECTION_STARTED){
printf("connection started!\n");
}else if(status==CONNECTION_MADE){
printf(green "Connection made.\n" rst);	
}else{printf(yellow "connecting...\n");}
mainloop(conn);
PQfinish(conn);
conn=NULL;
printf(green "***bye!***\n" rst);
return (KORE_RESULT_OK);
}
void exitclean(PGconn*conn){
printf(yellow "exitclean(conn) occured.\n");

PQfinish(conn);
done=1;
conn=NULL;
//exit(1);
//return (KORE_RESULT_OK)
}
int i=0;
static void han_sig(int n){
//atexit(mumu)
i++;
printf("n: %d\n",n);
printf(yellow "han_sig SIGINT occured.\n" rst);
done=1;	
if(conn !=NULL)PQfinish(conn);
conn=NULL;
//muka()
//usleep(100);
if(i==3)exit(0);
}
void foo(int n){
printf(yellow "FOO occured.\n" rst);
if(conn !=NULL){
printf("conn is not null in foo\n");done=1;PQfinish(conn);conn=NULL;
}else{printf("conn is null in foo\n");done=1;}
//usleep(10000000);
}
void mainloop(PGconn*conn){
fd_set rfds,wfds;
int retval;
int sock;
int connected=0;
	
int u=PQsetnonblocking(conn,1);
printf("non blocking: %d\n",u);
int ud=PQisnonblocking(conn);
// 1 nonbl, 0 bl
printf("is non blocking? :%d\n",ud);

sock=PQsocket(conn);	
if(sock<0){
printf("postgres socket is gone\n");
exitclean(conn);
}
					
PostgresPollingStatusType connstatus;
//struct sigaction *s;
//s.sa_handler=sigterm;
sigset_t blo,unblo;
sigemptyset(&blo);
sigaddset(&blo,SIGUSR1);
sigfillset(&unblo);
sigdelset(&unblo,SIGUSR1);
sigprocmask(SIG_BLOCK,&blo,NULL);
//signal(SIGUSR1,foo);
while(!done){
printf("*WHILE_LOOP*\n");
//usleep(20000);
//FD_ZERO(&rfds);
//FD_ZERO(&wfds);
if(!connected){
connstatus=PQconnectPoll(conn);
switch(connstatus){
case PGRES_POLLING_FAILED:
fprintf(stderr,"pgconn failed %s\n",PQerrorMessage(conn));
done=1;
exitclean(conn);
break;
case PGRES_POLLING_WRITING:
printf("PGRES_POLLING_WRITING\n");
//	FD_SET(sock,&wfds);
break;
case PGRES_POLLING_READING:
printf("PGRES_POLLING_READING\n");
FD_SET(sock,&rfds);
break;
case PGRES_POLLING_OK:
printf(green "PGRES_POLLING_OK\n" rst);
connected=1;
initlisten(conn);
break;
}
}else{
if(connstatus==PGRES_POLLING_OK){printf(red "poll OK\n" rst);
printf(green " done: %d\n" rst,done);
}else if(connstatus==PGRES_POLLING_FAILED){
		printf(red "pgres_polling_failed\n" rst);
		}else{printf(green "DEFALTI\n" rst);}
	}


if(connected){
printf("CONNECTED=true\n");

if(fuck==1){
//FD_ZERO(&rfds);
//FD_SET(sock,&rfds);
printf(green "FUCK?: %d\n" rst,fuck);
}
/*if(FD_ISSET(sock,&rfds)){
printf(yellow "DO HANDLE PG READ: retval: %d socket: %d\n" rst,retval,sock);
//if(retval==-1){done=1;exitclean(conn);}
handlepgread(conn);
}
*/ 
}
printf("BEFORE SELECT\n");
//retval=pselect(sock+1,&rfds,NULL,NULL,NULL,&blo);
retval=select(sock+1,&rfds,NULL,NULL,NULL);
//signal(SIGUSR1,foo);
printf("after select\n");
switch(retval){
case -1:
//perror("select failed\n");
if(errno==EINTR){
printf(red "EINTR occurred.\n");
}
printf(red "done??\n" rst);
connected=0;
//drug=1;
done=1;
break;
case 0:
printf("CASE 0\n");
break;
default:
if(!connected){
printf("Not connected.\n");
break;
}
if(FD_ISSET(sock,&rfds)){printf(yellow "DO HANDLE PG READ: retval: %d %d\n" rst,retval,sock);handlepgread(conn);}
printf("default\n");
}
//break;
printf("end of while\n");
}
}

void initlisten(PGconn*conn){
printf("Entering initlisten()\n");
char*quotedchannel=PQescapeIdentifier(conn,listenchannel,strlen(listenchannel));
char*query;
asprintf(&query,"LISTEN %s",quotedchannel);
int qs=PQsendQuery(conn,"LISTEN revents;LISTEN on_coders"/*query*/);
PQfreemem(quotedchannel);
free(query);
if(!qs){
fprintf(stderr,"failed to send query: %s\n",PQerrorMessage(conn));
return;
}
}
void handlepgread(PGconn*conn){
printf("entering handlepgread(conn)\n");
PGnotify*notify;
PGresult*res;
if(!PQconsumeInput(conn)){
	printf("hhhh\n");
fprintf(stderr,"failed to consume input: %s\n",PQerrorMessage(conn));
//done=1;
//exitclean(conn);
return;
}
printf("before result\n");


while((res=PQgetResult(conn)) !=NULL){
if(PQresultStatus(res) !=PGRES_COMMAND_OK){
fprintf(stderr,"result err: %s\n",PQerrorMessage(conn));
PQclear(res);
return;
}

printf("got result: %s\n",PQresStatus(PQresultStatus(res)));
printf("returns rows: %d\n",PQntuples(res));
printf("cols: %d\n",PQnfields(res));
printf("cmd status: %s\n",PQcmdStatus(res));
fuck=1;
PQclear(res);
}

printf("before notify\n");
while(notify=PQnotifies(conn)){
fprintf(stderr,yellow "notify of %s received from backend pid %d ,extra: %s\n" rst, notify->relname, notify->be_pid, notify->extra);
PQfreemem(notify);
}
printf("END\n");
}
