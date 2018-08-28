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

#include <sys/param.h>
#include <sys/queue.h>

#include <libpq-fe.h>
#include <pg_config.h>

#include "kore.h"

#if !defined(KORE_NO_HTTP)
#include "http.h"
#endif

#include "pgsql.h"

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"

int SUPERDB=0;int SUPERSUKA=0;
struct pgsql_wait {
	struct kore_pgsql	*pgsql;
	TAILQ_ENTRY(pgsql_wait)	list;
};

struct pgsql_job {
	struct kore_pgsql	*pgsql;
	TAILQ_ENTRY(pgsql_job)	list;
};

#define PGSQL_CONN_MAX		2
#define PGSQL_CONN_FREE		0x01
#define PGSQL_LIST_INSERTED	0x0100
#define PGSQL_QUEUE_LIMIT	1000

static void	pgsql_queue_wakeup(void);
static void	pgsql_cancel(struct kore_pgsql *);
static void	pgsql_set_error(struct kore_pgsql *, const char *);
static void	pgsql_queue_add(struct kore_pgsql *);
static void	pgsql_queue_remove(struct kore_pgsql *);
static void	pgsql_conn_release(struct kore_pgsql *);
static void	pgsql_conn_cleanup(struct pgsql_conn *);
static void	pgsql_read_result(struct kore_pgsql *);
static void	pgsql_schedule(struct kore_pgsql *);

static struct pgsql_conn	*pgsql_conn_create(struct kore_pgsql *,
				    struct pgsql_db *);
static struct pgsql_conn	*pgsql_conn_next(struct kore_pgsql *,
				    struct pgsql_db *);

static struct kore_pool			pgsql_job_pool;
static struct kore_pool			pgsql_wait_pool;
static TAILQ_HEAD(, pgsql_conn)		pgsql_conn_free;
static TAILQ_HEAD(, pgsql_wait)		pgsql_wait_queue;
static LIST_HEAD(, pgsql_db)		pgsql_db_conn_strings;

u_int32_t	pgsql_queue_count = 0;
u_int16_t	pgsql_conn_max = PGSQL_CONN_MAX;
u_int32_t	pgsql_queue_limit = PGSQL_QUEUE_LIMIT;

void
kore_pgsql_sys_init(void)
{
	TAILQ_INIT(&pgsql_conn_free);
	TAILQ_INIT(&pgsql_wait_queue);
	LIST_INIT(&pgsql_db_conn_strings);

	kore_pool_init(&pgsql_job_pool, "pgsql_job_pool",
	    sizeof(struct pgsql_job), 100);
	kore_pool_init(&pgsql_wait_pool, "pgsql_wait_pool",
	    sizeof(struct pgsql_wait), pgsql_queue_limit);
}

void
kore_pgsql_sys_cleanup(void)
{
	struct pgsql_conn	*conn, *next;

	kore_pool_cleanup(&pgsql_job_pool);
	kore_pool_cleanup(&pgsql_wait_pool);

	for (conn = TAILQ_FIRST(&pgsql_conn_free); conn != NULL; conn = next) {
		next = TAILQ_NEXT(conn, list);
		//printf("kore_pgsql_sys_cleanup from pgsql_conn_free\n");
		pgsql_conn_cleanup(conn);
	}
}

void
kore_pgsql_init(struct kore_pgsql *pgsql)
{
	memset(pgsql, 0, sizeof(*pgsql));
	pgsql->state = KORE_PGSQL_STATE_INIT;
}

int
kore_pgsql_setup(struct kore_pgsql *pgsql, const char *dbname, int flags)
{
	struct pgsql_db		*db;

	if ((flags & KORE_PGSQL_ASYNC) && (flags & KORE_PGSQL_SYNC)) {
		pgsql_set_error(pgsql, "invalid query init parameters");
		return (KORE_RESULT_ERROR);
	}

	if (flags & KORE_PGSQL_ASYNC) {
		if (pgsql->req == NULL && pgsql->cb == NULL) {
			printf(red "\n nothing was bound\n\n" rst);
			pgsql_set_error(pgsql, "nothing was bound");
			return (KORE_RESULT_ERROR);
		}
	}

	db = NULL;
	pgsql->flags |= flags;

	LIST_FOREACH(db, &pgsql_db_conn_strings, rlist) {
		if (!strcmp(db->name, dbname))
			break;
	}

	if (db == NULL) {
		printf(red "no database found in names\n" rst);
		pgsql_set_error(pgsql, "no database found");
		return (KORE_RESULT_ERROR);
	}

	if ((pgsql->conn = pgsql_conn_next(pgsql, db)) == NULL){
		printf(red "\n\n*** return kore_result error from setup error\n" rst);
		return (KORE_RESULT_ERROR);}

	if (pgsql->flags & KORE_PGSQL_ASYNC) {
		//printf("get job\n");
		pgsql->conn->job = kore_pool_get(&pgsql_job_pool);
		pgsql->conn->job->pgsql = pgsql;
	}else{
	//printf(red "no fucking flags in async\n" rst);
	}

	return (KORE_RESULT_OK);
}

#if !defined(KORE_NO_HTTP)
void
kore_pgsql_bind_request(struct kore_pgsql *pgsql, struct http_request *req)
{
	if (pgsql->req != NULL || pgsql->cb != NULL)
		fatal("kore_pgsql_bind_request: already bound");

	pgsql->req = req;
	pgsql->flags |= PGSQL_LIST_INSERTED;

	LIST_INSERT_HEAD(&(req->pgsqls), pgsql, rlist);
}
#endif

void
kore_pgsql_bind_callback(struct kore_pgsql *pgsql,
    void (*cb)(struct kore_pgsql *, void *), void *arg)
{
	if (pgsql->req != NULL)
		fatal("kore_pgsql_bind_callback: already bound");

	if (pgsql->cb != NULL)
		fatal("kore_pgsql_bind_callback: already bound");

	pgsql->cb = cb;
	pgsql->arg = arg;
}

int
kore_pgsql_query(struct kore_pgsql *pgsql, const char *query)
{
	if (pgsql->conn == NULL) {
		pgsql_set_error(pgsql, "no connection was set before query");
		return (KORE_RESULT_ERROR);
	}

	if (pgsql->flags & KORE_PGSQL_SYNC) {
		pgsql->result = PQexec(pgsql->conn->db, query);
		if ((PQresultStatus(pgsql->result) != PGRES_TUPLES_OK) &&
		    (PQresultStatus(pgsql->result) != PGRES_COMMAND_OK)) {
			pgsql_set_error(pgsql, PQerrorMessage(pgsql->conn->db));
			return (KORE_RESULT_ERROR);
		}

		pgsql->state = KORE_PGSQL_STATE_DONE;
	} else {
		if (!PQsendQuery(pgsql->conn->db, query)) {
			pgsql_set_error(pgsql, PQerrorMessage(pgsql->conn->db));
			return (KORE_RESULT_ERROR);
		}
//kore_log(LOG_INFO, green "*** BEFORE PGSQL_SCHEDULE(PGSQL)! IN QUERY SENDING!!! ***" rst);
		pgsql_schedule(pgsql);
	}

	return (KORE_RESULT_OK);
}

int
kore_pgsql_v_query_params(struct kore_pgsql *pgsql,
    const char *query, int result, int count, va_list args)
{
	u_int8_t	i;
	char		**values;
	int		*lengths, *formats, ret;

	if (pgsql->conn == NULL) {
		pgsql_set_error(pgsql, "no connection was set before query");
		return (KORE_RESULT_ERROR);
	}

	if (count > 0) {
		lengths = kore_calloc(count, sizeof(int));
		formats = kore_calloc(count, sizeof(int));
		values = kore_calloc(count, sizeof(char *));

		for (i = 0; i < count; i++) {
			values[i] = va_arg(args, void *);
			lengths[i] = va_arg(args, int);
			formats[i] = va_arg(args, int);
		}
	} else {
		lengths = NULL;
		formats = NULL;
		values = NULL;
	}

	ret = KORE_RESULT_ERROR;

	if (pgsql->flags & KORE_PGSQL_SYNC) {
		pgsql->result = PQexecParams(pgsql->conn->db, query, count,
		    NULL, (const char * const *)values, lengths, formats,
		    result);

		if ((PQresultStatus(pgsql->result) != PGRES_TUPLES_OK) &&
		    (PQresultStatus(pgsql->result) != PGRES_COMMAND_OK)) {
			pgsql_set_error(pgsql, PQerrorMessage(pgsql->conn->db));
			goto cleanup;
		}

		pgsql->state = KORE_PGSQL_STATE_DONE;
	} else {
		if (!PQsendQueryParams(pgsql->conn->db, query, count, NULL,
		    (const char * const *)values, lengths, formats, result)) {
			pgsql_set_error(pgsql, PQerrorMessage(pgsql->conn->db));
			goto cleanup;
		}

		pgsql_schedule(pgsql);
	}

	ret = KORE_RESULT_OK;

cleanup:
	kore_free(values);
	kore_free(lengths);
	kore_free(formats);

	return (ret);
}

int
kore_pgsql_query_params(struct kore_pgsql *pgsql,
    const char *query, int result, int count, ...)
{
	int		ret;
	va_list		args;

	va_start(args, count);
	ret = kore_pgsql_v_query_params(pgsql, query, result, count, args);
	va_end(args);

	return (ret);
}

int
kore_pgsql_register(const char *dbname, const char *connstring)
{
	struct pgsql_db		*pgsqldb;

	LIST_FOREACH(pgsqldb, &pgsql_db_conn_strings, rlist) {
		if (!strcmp(pgsqldb->name, dbname))
			return (KORE_RESULT_ERROR);
	}

	pgsqldb = kore_malloc(sizeof(*pgsqldb));
	pgsqldb->name = kore_strdup(dbname);
	pgsqldb->conn_count = 0;
	pgsqldb->conn_max = pgsql_conn_max;
	pgsqldb->conn_string = kore_strdup(connstring);
	LIST_INSERT_HEAD(&pgsql_db_conn_strings, pgsqldb, rlist);

	return (KORE_RESULT_OK);
}

void
kore_pgsql_handle(void *c, int err)
{
	//printf("*** ENTERING KORE_PGSQL_HANDLE ***\n");
	//printf("*** SUPERDB!!! : %d ****\n",SUPERDB);
	struct kore_pgsql	*pgsql;
	struct pgsql_conn	*conn = (struct pgsql_conn *)c;

	if (err) {
		pgsql_conn_cleanup(conn);
		return;
	}

	pgsql = conn->job->pgsql;

	if (!PQconsumeInput(conn->db)) {
		//printf("*** NOT CONSUME INPUT ***\n");
		pgsql->state = KORE_PGSQL_STATE_ERROR;
		pgsql->error = kore_strdup(PQerrorMessage(conn->db));
	} 
	else {
		//printf("***before pgsql_read_result***\n");
		pgsql_read_result(pgsql);
	}

	if (pgsql->state == KORE_PGSQL_STATE_WAIT) {
		//printf("***state wait in handle pgsql***\n");
#if !defined(KORE_NO_HTTP)
		if (pgsql->req != NULL)
			http_request_sleep(pgsql->req);
#endif
		if (pgsql->cb != NULL)
			pgsql->cb(pgsql, pgsql->arg);
	} else {
	//	printf("*** else no wait state***\n");
#if !defined(KORE_NO_HTTP)
		if (pgsql->req != NULL)
			http_request_wakeup(pgsql->req);
#endif
		if (pgsql->cb != NULL){
		//	printf("***before  HANDLE CB ***\n");
			pgsql->cb(pgsql, pgsql->arg);
		//	printf("***after handle cb***\n");
			}
	}
}

void
kore_pgsql_continue(struct kore_pgsql *pgsql)
{
	//printf(" *** kore_pgsql_continue CONTINUE ****\n");
	if (pgsql->error) {
		kore_free(pgsql->error);
		pgsql->error = NULL;
	}

	if (pgsql->result) {
	//	printf("***it looks like result in continue***\n");
		PQclear(pgsql->result);
		pgsql->result = NULL;
	}

	switch (pgsql->state) {
	case KORE_PGSQL_STATE_INIT:
	case KORE_PGSQL_STATE_WAIT:
	//printf("***state init and wait in continue***\n");
		break;
	case KORE_PGSQL_STATE_DONE:
	//printf("***state done in continue***\n");
#if !defined(KORE_NO_HTTP)
		if (pgsql->req != NULL)
			http_request_wakeup(pgsql->req);
#endif
//printf("*** before conn release ***\n");
		pgsql_conn_release(pgsql);
//printf("*** AFTER PGSQL_CONN_RELEASE ***\n");
		break;
	case KORE_PGSQL_STATE_ERROR:
	case KORE_PGSQL_STATE_RESULT:
	case KORE_PGSQL_STATE_NOTIFY:
	case KORE_PGSQL_STATE_COMMANDOK:
	
	//printf("***before kore pgsql handle in continue***\n");
		kore_pgsql_handle(pgsql->conn, 0);
		break;
	default:
		fatal("*** Unknown pgsql state_3 *** %d", pgsql->state);
	}
}

void
kore_pgsql_cleanup(struct kore_pgsql *pgsql)
{
	pgsql_queue_remove(pgsql);
//printf("kore_pgsql_cleanup\n");
	if (pgsql->result != NULL)
		PQclear(pgsql->result);

	if (pgsql->error != NULL)
		kore_free(pgsql->error);

	if (pgsql->conn != NULL)
		pgsql_conn_release(pgsql);

	pgsql->result = NULL;
	pgsql->error = NULL;
	pgsql->conn = NULL;

	if (pgsql->flags & PGSQL_LIST_INSERTED) {
		//printf("flag pgsql_list_inserted detected\n");
		LIST_REMOVE(pgsql, rlist);
		pgsql->flags &= ~PGSQL_LIST_INSERTED;
	}
}

void
kore_pgsql_logerror(struct kore_pgsql *pgsql)
{
	kore_log(LOG_NOTICE, "pgsql error: %s",
	    (pgsql->error) ? pgsql->error : "unknown");
}

int
kore_pgsql_ntuples(struct kore_pgsql *pgsql)
{
	return (PQntuples(pgsql->result));
}

int
kore_pgsql_nfields(struct kore_pgsql *pgsql)
{
	return (PQnfields(pgsql->result));
}

int
kore_pgsql_getlength(struct kore_pgsql *pgsql, int row, int col)
{
	return (PQgetlength(pgsql->result, row, col));
}

char *
kore_pgsql_fieldname(struct kore_pgsql *pgsql, int field)
{
	return (PQfname(pgsql->result, field));
}

char *
kore_pgsql_getvalue(struct kore_pgsql *pgsql, int row, int col)
{
	return (PQgetvalue(pgsql->result, row, col));
}

static struct pgsql_conn *
pgsql_conn_next(struct kore_pgsql *pgsql, struct pgsql_db *db)
{
	//printf("***entering pgsql_conn_next\n");
	PGTransactionStatusType		state;
	struct pgsql_conn		*conn;
	struct kore_pgsql		rollback;

rescan:
	conn = NULL;

	TAILQ_FOREACH(conn, &pgsql_conn_free, list) {
		if (!(conn->flags & PGSQL_CONN_FREE)){
			fatal("got a pgsql connection that was not free?");
			//printf("got a pgsql connection that was not free?\n");
		}
		if (!strcmp(conn->name, db->name))
			break;
	}

	if (conn != NULL) {
		//printf(yellow "conn is not NULL, before pqtransactionstatus\n" rst);
		state = PQtransactionStatus(conn->db);
		if (state == PQTRANS_INERROR) {
			kore_log(LOG_INFO,red "pqtrans_inerror\n" rst);
			conn->flags &= ~PGSQL_CONN_FREE;
			TAILQ_REMOVE(&pgsql_conn_free, conn, list);

			kore_pgsql_init(&rollback);
			rollback.conn = conn;
			rollback.flags = KORE_PGSQL_SYNC;

			if (!kore_pgsql_query(&rollback, "ROLLBACK")) {
				kore_log(LOG_INFO, red "*** NO ROLLBACK ***" rst);
				kore_pgsql_logerror(&rollback);
				kore_pgsql_cleanup(&rollback);
				pgsql_conn_cleanup(conn);
			} else {
			//	printf("*** CLEANUP ROLLBACK!***\n");
				kore_pgsql_cleanup(&rollback);
			}
//kore_log(LOG_INFO, yellow "*** GOTO RESCAN!!! ***" rst);
			goto rescan;
		}
	}

	if (conn == NULL) {
		//kore_log(LOG_INFO, green "*** CONN is NULL! ***\n" rst);
		if (db->conn_max != 0 &&  db->conn_count >= db->conn_max) {
				printf(red "db->conn_count: %d\n" rst,db->conn_count);
			if ((pgsql->flags & KORE_PGSQL_ASYNC) && pgsql_queue_count < pgsql_queue_limit) {
					printf(red "pgsql_queue_count: %d\n" rst,pgsql_queue_count);
				pgsql_queue_add(pgsql);
			} else {
				kore_log(LOG_INFO, yellow "\n\n\n\n no available connection \n\n\n\n" rst);
				pgsql_set_error(pgsql,"no available connection");
			}
//printf(red "***returning NULL from here ***\n\n");
			return (NULL);
		}

		if ((conn = pgsql_conn_create(pgsql, db)) == NULL){
			printf(red "\n\n\n CONNECTION CREATE IS NULL!!! returning null ***\n\n\n\n" rst);
			return (NULL);
		}
	}

	conn->flags &= ~PGSQL_CONN_FREE;
	TAILQ_REMOVE(&pgsql_conn_free, conn, list);
//printf("connection created!\n");
	return (conn);
}

static void
pgsql_set_error(struct kore_pgsql *pgsql, const char *msg)
{
	if (pgsql->error != NULL)
		kore_free(pgsql->error);

	pgsql->error = kore_strdup(msg);
	pgsql->state = KORE_PGSQL_STATE_ERROR;
}

static void
pgsql_schedule(struct kore_pgsql *pgsql)
{
	//kore_log(LOG_INFO, yellow "*** pgsql_schedule(pgsql) Entering!!! ***" rst);
	int		fd;

	fd = PQsocket(pgsql->conn->db);
	if (fd < 0)
		fatal("PQsocket returned < 0 fd on open connection");
//kore_log(LOG_INFO, red "*** FD IN SCHEDULE: %d ***" rst,fd);
	kore_platform_schedule_read(fd, pgsql->conn);
	pgsql->state = KORE_PGSQL_STATE_WAIT;
	pgsql->flags |= KORE_PGSQL_SCHEDULED;

#if !defined(KORE_NO_HTTP)
	if (pgsql->req != NULL)
		http_request_sleep(pgsql->req);
#endif
	if (pgsql->cb != NULL)
		pgsql->cb(pgsql, pgsql->arg);
}

static void
pgsql_queue_add(struct kore_pgsql *pgsql)
{
	//printf("*** ENTERING QUEUEADD ***\n");
	struct pgsql_wait	*pgw;

#if !defined(KORE_NO_HTTP)
	if (pgsql->req != NULL)
		http_request_sleep(pgsql->req);
#endif

if(pgsql->conn){
if(pgsql->conn==NULL){
printf(red "\n\n\n\n\n\n ** ERROR!!!! pgsql->conn is NULL!!! que add***\n\n\n"rst);
}	
}

	pgw = kore_pool_get(&pgsql_wait_pool);
	pgw->pgsql = pgsql;
//printf("inserting into wait queue\n");
	pgsql_queue_count++;
	TAILQ_INSERT_TAIL(&pgsql_wait_queue, pgw, list);
//	printf("***  END QUEUEADD***\n");
}

static void
pgsql_queue_remove(struct kore_pgsql *pgsql)
{
	//printf("*** ENTERING QUEUE REMOVE  ***\n");
	struct pgsql_wait	*pgw, *next;

	for (pgw = TAILQ_FIRST(&pgsql_wait_queue); pgw != NULL; pgw = next) {
		next = TAILQ_NEXT(pgw, list);
		if (pgw->pgsql != pgsql)
			continue;

		pgsql_queue_count--;
	//	printf("removing from wait queue\n");
		TAILQ_REMOVE(&pgsql_wait_queue, pgw, list);
		kore_pool_put(&pgsql_wait_pool, pgw);
		return;
	}
}

static void
pgsql_queue_wakeup(void)
{
	
	//printf("*** ENTERING WAKEUP ***\n");
	
	struct pgsql_wait	*pgw, *next;
//printf("*** BEFORE LOOP FOR ***\n");
	for (pgw = TAILQ_FIRST(&pgsql_wait_queue); pgw != NULL; pgw = next) {
		next = TAILQ_NEXT(pgw, list);
//printf("*** AFTER NEXT OF WAIT QUEUE ***\n");
#if !defined(KORE_NO_HTTP)
		if (pgw->pgsql->req != NULL) {
			//printf("*** REQ IS NOT NULL***\n");
			if (pgw->pgsql->req->flags & HTTP_REQUEST_DELETE) {
				pgsql_queue_count--;
				TAILQ_REMOVE(&pgsql_wait_queue, pgw, list);
				kore_pool_put(&pgsql_wait_pool, pgw);
				continue;
			}

			http_request_wakeup(pgw->pgsql->req);
		}
#endif
//printf("*** BEFORE CB IN WAKEUP ***\n");
		if (pgw->pgsql->cb != NULL)
pgw->pgsql->cb(pgw->pgsql, pgw->pgsql->arg);
//printf("*** AFTER CB WAKEUP ***\n");
		pgsql_queue_count--;
		//printf("*** BEFORE REMV QUE from wait queue ***\n");
		TAILQ_REMOVE(&pgsql_wait_queue, pgw, list);
		//printf("*** BEFORE POOL PUT in wakup ***\n");
		kore_pool_put(&pgsql_wait_pool, pgw);
		return;
	}
	 
}

static struct pgsql_conn *
pgsql_conn_create(struct kore_pgsql *pgsql, struct pgsql_db *db)
{
	int  bdone=0;
	int connected=0;
	int retval;
	struct pgsql_conn	*conn;

	if (db == NULL || db->conn_string == NULL)
		fatal("pgsql_conn_create: no connection string");

	db->conn_count++;

	conn = kore_malloc(sizeof(*conn));
	conn->job = NULL;
	conn->flags = PGSQL_CONN_FREE;
	conn->type = KORE_TYPE_PGSQL_CONN;
	conn->name = kore_strdup(db->name);
	TAILQ_INSERT_TAIL(&pgsql_conn_free, conn, list);

	//conn->db = PQconnectdb(db->conn_string);
	conn->db=PQconnectStart(db->conn_string);
	
	ConnStatusType status=PQstatus(conn->db);
	if(status==CONNECTION_BAD){
		kore_log(LOG_INFO, red "\n\n*** CONNECTION_BAD!!! ***\n\n\n" rst);
		pgsql_set_error(pgsql, PQerrorMessage(conn->db));
		pgsql_conn_cleanup(conn);
		return (NULL);
	}else if(status==CONNECTION_STARTED){
	//kore_log(LOG_INFO, green "*** CONNECTION STARTED! ***" rst);	
	}else if(status==CONNECTION_MADE){
	//kore_log(LOG_INFO, green "*** CONNECTION_MADE! ***\n" rst);	
	}else{
	printf("*** UNKNOWN ONNECTION ***\n");
	}
	fd_set rfds;
	//int u=
	PQsetnonblocking(conn->db,1);
	//printf("*** set non block conn->db: %d ****\n",u);
	
	PostgresPollingStatusType connstatus;
	int fd=PQsocket(conn->db);
	if(fd<0){printf(red "\n\n\n\n*** ERROR!!! FD IS: %d\n\n\n\n" rst,fd);}
	while(!bdone){
	if(!connected){
	connstatus=PQconnectPoll(conn->db);
	if(connstatus==PGRES_POLLING_FAILED){
	kore_log(LOG_INFO, red "*** PGRES_POLLING_FAILED! ***" rst);
	pgsql_set_error(pgsql, PQerrorMessage(conn->db));
	pgsql_conn_cleanup(conn);
	bdone=1;
	return (NULL);	
	}else if(connstatus==PGRES_POLLING_WRITING){
	//kore_log(LOG_INFO, red "*** PGRES_POLLING_WRITING! ***" rst);	
	//kore_log(LOG_INFO, green "*** NOTHING TO BE DONE HERE??? ***" rst);
	}else if(connstatus==PGRES_POLLING_READING){
	//printf("*** PGRES_POLLING_READING! ***\n");	
	//pgsql_schedule(pgsql);
	//int fd=PQsocket(conn->db);
	//kore_platform_schedule_read(fd, conn->db);
	FD_SET(fd,&rfds);
	SUPERSUKA+=1;
	}else if(connstatus==PGRES_POLLING_OK){
	//kore_log(LOG_INFO, green "*** PGRES_POLLING_OK! ***" rst);	
	//kore_log(LOG_INFO, yellow "*** SUPERSUKA(how much was poll reading): %d ***" rst,SUPERSUKA);
	//kore_log(LOG_INFO,red "*** SHOW ME FD! %d ***" rst, fd);
	SUPERSUKA=0;
	bdone=1;
	connected=1;
	SUPERDB+=1;
	break;
	}else{printf("*** UNKNOWN PGRES STATE ***\n");}	
	}//connected
	// the fuck knows what I'm doing right here
	// is it really db connection here "asynchronous"??? Or is it all  about just like PQconnectdb???
	retval=select(fd+1,&rfds,NULL,NULL,NULL);
	//kore_log(LOG_INFO, green"*** retval : %d ***n" rst,retval);
	if(retval < 0){printf(red "\n\n\n *** RETVAL IN SELECTL %d ****\n\n\n" rst, retval);}
	//kore_log(LOG_INFO, red "*** a fd : %d ***" rst,fd);
	if(!connected){
	//printf(red "\n\n *** NOT CONNECTED!!!! ***\n" rst);
		//break;
		}
	
}//while loop
	//kk
	/*
	if (conn->db == NULL || (PQstatus(conn->db) != CONNECTION_OK)) {
		pgsql_set_error(pgsql, PQerrorMessage(conn->db));
		pgsql_conn_cleanup(conn);
		return (NULL);
	}*/
	if (conn->db == NULL/* || (PQstatus(conn->db) != CONNECTION_OK)*/) {
		kore_log(LOG_INFO, red "\n\n\n\n\n\n *** ACHTUNG!!! CONN->DB IS NULL!!! ***\n\n\n\n\n" rst);
		pgsql_set_error(pgsql, PQerrorMessage(conn->db));
		pgsql_conn_cleanup(conn);
		return (NULL);
	}
//printf("*** RETURNING NOW CONN INSTANCE! in conn create***\n");
	return (conn);
}

static void
pgsql_conn_release(struct kore_pgsql *pgsql)
{
	//printf("***RELEASE CONN***\n");
	int		fd;
	PGresult	*result;

	if (pgsql->conn == NULL){
	//	printf("***pgsql->conn is NULL ***\n");
		return;
	}

	/* Async query cleanup */
	if (pgsql->flags & KORE_PGSQL_ASYNC) {
		if (pgsql->flags & KORE_PGSQL_SCHEDULED) {
		//	printf("*** flag sceduled *** \n");
			fd = PQsocket(pgsql->conn->db);
		//	printf("fd before kore_platform_disable_read: %d\n",fd);
			//printf(red "*** pgsql->conn->name : %s\n***" rst, pgsql->conn->name);
			kore_log(LOG_INFO,red "fd is: %d" rst,fd);
				kore_platform_disable_read(fd);
			/*
			 * if(strcmp(pgsql->conn->name,"fucker")){
				kore_log(LOG_INFO, red "not fucker here" rst);
				kore_log(LOG_INFO,yellow "fd is: %d" rst,fd);
				kore_platform_disable_read(fd);
				}else{
					kore_log(LOG_INFO,green "fucker must be here" rst);
					kore_log(LOG_INFO,red "fd is: %d" rst,fd);
				//kore_platform_disable_read(fd);
				}
*/
			if (pgsql->state != KORE_PGSQL_STATE_DONE){
			//	printf(" *** state not done ***\n");
				pgsql_cancel(pgsql);
			}
		}
		kore_pool_put(&pgsql_job_pool, pgsql->conn->job);
	}

	/* Drain just in case. */
	while ((result = PQgetResult(pgsql->conn->db)) != NULL){
		//printf("*** THERE ARE RESULTS!!!! By now PQclear that***\n");
		PQclear(result);
	}

	pgsql->conn->job = NULL;
	pgsql->conn->flags |= PGSQL_CONN_FREE;
	TAILQ_INSERT_TAIL(&pgsql_conn_free, pgsql->conn, list);

	pgsql->conn = NULL;
	pgsql->state = KORE_PGSQL_STATE_COMPLETE;
//printf("*** after state complete ***\n");
if (pgsql->cb != NULL)pgsql->cb(pgsql, pgsql->arg);
//printf("*** Before wakeup ****\n");
	pgsql_queue_wakeup();
//	printf("*** end conn release *** \n");
}

static void
pgsql_conn_cleanup(struct pgsql_conn *conn)
{
	//printf("***entering pgsql_conn_cleanup()***\n");
	struct kore_pgsql	*pgsql;
	struct pgsql_db		*pgsqldb;

	if (conn->flags & PGSQL_CONN_FREE){
	//	printf("***remove from connection free***\n");
		TAILQ_REMOVE(&pgsql_conn_free, conn, list);
	}

	if (conn->job) {
	//	printf("***CONN JOB!!***\n");
		pgsql = conn->job->pgsql;
#if !defined(KORE_NO_HTTP)
		if (pgsql->req != NULL)
			http_request_wakeup(pgsql->req);
#endif
		pgsql->conn = NULL;
		pgsql_set_error(pgsql, PQerrorMessage(conn->db));

		kore_pool_put(&pgsql_job_pool, conn->job);
		conn->job = NULL;
	}

	if (conn->db != NULL)
		PQfinish(conn->db);
//printf("*** after finish ***\n");
	LIST_FOREACH(pgsqldb, &pgsql_db_conn_strings, rlist) {
		if (strcmp(pgsqldb->name, conn->name)) {
			pgsqldb->conn_count--;
			break;
		}
	}

	kore_free(conn->name);
	kore_free(conn);
}

static void
pgsql_read_result(struct kore_pgsql *pgsql)
{
	//kore_log(LOG_INFO, green "\n\n*** SUPERDB: %d ***\n\n" rst, SUPERDB);
	//printf("entering pgsql read result***()\n");
	PGnotify	*notify;
//printf(red "before busy***\n" rst);
if(pgsql->conn){
//printf(red "YES! pgsql->conn ***\n" rst);
}else{
//printf(red "NO! pgsql->conn!!!! ***\n" rst);
if(pgsql->conn==NULL){
printf(red "\n\n\n\n\nERROR!!! YES IS NULL pgsql->conn returning!!!! One need to destroy it **** \n\n\n\n\n" rst);return;}
	}
if(pgsql->conn->db){
//printf(red "YES! pgsql->conn->db!!!  ***\n" rst);
}else{printf(red "NO!!!! pgsql->conn->db !!!! ***\n" rst);}
	if (PQisBusy(pgsql->conn->db)) {
		pgsql->state = KORE_PGSQL_STATE_WAIT;
		kore_log(LOG_INFO, red "*** Oh, nooo!!! The busy wait occured! returning! ***\n" rst);
		return;
	}
	
//PQconsumeInput(pgsql->conn->db);

	while ((notify = PQnotifies(pgsql->conn->db)) != NULL) {
		pgsql->state = KORE_PGSQL_STATE_NOTIFY;
		pgsql->notify.extra = notify->extra;
		pgsql->notify.channel = notify->relname;

		if (pgsql->cb != NULL)
			pgsql->cb(pgsql, pgsql->arg);

		PQfreemem(notify);
	}
/*
	pgsql->result = PQgetResult(pgsql->conn->db);
	*/
	if (pgsql->result == NULL) {
		//printf(green "pgsql->result is NULL\n" rst);
		//pgsql->state = KORE_PGSQL_STATE_DONE;
		//return;
	}
	
	//PQconsumeInput(pgsql->conn->db);
	//printf(red "BEFORE WHILE LOOP RESULT***\n" rst);
while((pgsql->result=PQgetResult(pgsql->conn->db))!=NULL){
	//printf("while pgsql->result\n");
	/*if(PQresultStatus(pgsql->result)==PGRES_COMMAND_OK){
		printf("status is ok!\n");
		pgsql->state = KORE_PGSQL_STATE_DONE;
		//break;
		if (pgsql->cb != NULL){
			printf("comm on result: %s\n",PQcmdStatus(pgsql->result));
			//pgsql->cb(pgsql, pgsql->arg);
			
		}
		}*/
		
	//}
	
	
	switch (PQresultStatus(pgsql->result)) {
	case PGRES_COPY_OUT:
	case PGRES_COPY_IN:
	case PGRES_NONFATAL_ERROR:
	case PGRES_COPY_BOTH:
		break;
	case PGRES_COMMAND_OK:
	//printf("pgres_command is ok %p\n", (void*)pgsql);
	//printf("comm on result: %s\n",PQcmdStatus(pgsql->result));
	//	pgsql->state = KORE_PGSQL_STATE_DONE;
		pgsql->state=KORE_PGSQL_STATE_COMMANDOK;
		if(pgsql->req !=NULL){
		//printf("*** REQ3 NOT3 NULL3 return3 ***\n");
		return;
		}
	if (pgsql->cb != NULL) pgsql->cb(pgsql, pgsql->arg);
		break;
	case PGRES_TUPLES_OK:
#if PG_VERSION_NUM >= 90200
	case PGRES_SINGLE_TUPLE:
#endif
//printf("*** state result ***?\n");
		pgsql->state = KORE_PGSQL_STATE_RESULT;
		if (pgsql->req != NULL){return;}
		if (pgsql->cb != NULL) pgsql->cb(pgsql, pgsql->arg);
		PQclear(pgsql->result);
		pgsql->result = NULL;
	//	printf("*** after pqclear***\n");
		
		break;
	case PGRES_EMPTY_QUERY:
	case PGRES_BAD_RESPONSE:
	case PGRES_FATAL_ERROR:
	//printf("*** pgres fatal error ***\n");
		pgsql_set_error(pgsql, PQresultErrorMessage(pgsql->result));
	if (pgsql->cb != NULL) pgsql->cb(pgsql, pgsql->arg);
	if(pgsql->req !=NULL){
	//printf("*** some err. return req ***\n");
	return;
	}
		break;
	}
	//printf("***end of while***\n");
	
	//if(pgsql->cb !=NULL) pgsql->cb(pgsql,pgsql->arg);
}

//printf("*****************************\n");

if(pgsql->state==KORE_PGSQL_STATE_COMMANDOK){
	//printf("***immer now pgsql->state is COMMANDOK!***\n");
	pgsql->state=KORE_PGSQL_STATE_DONE;
	}else{
	//printf("***pgsql->state is not COMMANDOK!:%d \n",pgsql->state);
	if(pgsql->result==NULL){
	//printf("***pgsql->result is NULL***\n");
	pgsql->state=KORE_PGSQL_STATE_DONE;	
	if(pgsql->req !=NULL){
	//printf("***REQ NOT NULL return***\n");
	return;
	}else{
	//printf("***REQ IS NULL***\n");
	}
	}else{
	//printf("***pgsql->result is NOT NULL***");
	}
	}

//printf("*** END OF pgsql_read_result ***\n");
}

static void
pgsql_cancel(struct kore_pgsql *pgsql)
{
	printf("***entering pgsql_cancel***\n");
	PGcancel	*cancel;
	char		buf[256];

	if ((cancel = PQgetCancel(pgsql->conn->db)) != NULL) {
		if (!PQcancel(cancel, buf, sizeof(buf)))
			kore_log(LOG_ERR, "failed to cancel: %s", buf);
		PQfreeCancel(cancel);
	}
}
