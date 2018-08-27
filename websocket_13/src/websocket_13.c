#include <signal.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>
#include <kore/tasks.h>
//#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h> //PRIu64
//#include <sys/types.h>
#include "assets.h"
#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
const char*q_name="db";
const char*q_fucker="fucker";
const char*q_subscribe="subscribe";
const char*q_on_boss_job_fail="job_fails";

struct boss_fail{
char*jobid;	
};

struct kore_pgsql*for_fucker=NULL;// for listen notify
struct kore_timer*tim=NULL;
struct kore_pgsql*pgsql2=NULL;//for pgboss job scheduler

void boss_done(struct kore_pgsql*,char*,char*,char*);//,jobid,eri,erfolgi

enum {
	WOWA=7
};

static volatile sig_atomic_t blog_sig=-1;
void signal_handler(int);
void tick(void*,u_int64_t);

int	page(struct http_request *);

int init(int);

void on_notify(struct kore_pgsql*);

void db_state_change(struct kore_pgsql*,void*);

void db_query(struct kore_pgsql*,const char*, const char*);
void db_query_params(struct kore_pgsql*p,const char*qname,const char*str_query,int,int,...);
void db_results(struct kore_pgsql*,void*);

const char*das_nextJob_query="WITH nextJob as (SELECT id FROM pgboss.job WHERE state < 'active' \
AND name = ANY($1) AND startAfter < now() \
ORDER BY priority desc,createdOn,id LIMIT $2 FOR UPDATE SKIP LOCKED) UPDATE pgboss.job j SET state='active', \
startedOn=now(),retryCount=CASE WHEN state='retry' THEN retryCount + 1 ELSE retryCount END FROM nextJob \
WHERE j.id=nextJob.id RETURNING j.id, name, to_json(data::text)";

/*
 
WITH nextJob as (SELECT id FROM pgboss.job WHERE state < 'active' AND name = ANY('{jobbi,fucker}') AND startAfter < now() ORDER BY priority desc,createdOn,id LIMIT 1 FOR UPDATE SKIP LOCKED) UPDATE pgboss.job j SET state='active', startedOn=now(),retryCount=CASE WHEN state='retry' THEN retryCount + 1 ELSE retryCount END FROM nextJob WHERE j.id=nextJob.id RETURNING j.id, name, data;

// data string to json:

WITH nextJob as (SELECT id FROM pgboss.job WHERE state < 'active' AND name = ANY('{jobbi,fucker}') AND startAfter < now() ORDER BY priority desc,createdOn,id LIMIT 1 FOR UPDATE SKIP LOCKED) UPDATE pgboss.job j SET state='active',startedOn=now(),retryCount=CASE WHEN state='retry' THEN retryCount + 1 ELSE retryCount END FROM nextJob WHERE j.id=nextJob.id RETURNING j.id, name, to_json(data::text);



*/



int page_ws_connect(struct http_request*);
void websocket_connect(struct connection*);
void websocket_disconnect(struct connection*);
void websocket_message(struct connection*,u_int8_t,void*,size_t);
void han(void);
void connection_new(struct connection*);
void connection_del(struct connection*);

void han(){
printf(green "at exit occured.\n" rst);
if(tim !=NULL){
kore_timer_remove(tim);
tim=NULL;
}
//if(for_fucker !=NULL)kore_pgsql_cleanup(for_fucker);
if(for_fucker !=NULL){
kore_pgsql_continue(for_fucker);
for_fucker=NULL;
}
if(pgsql2 !=NULL){
kore_pgsql_cleanup(pgsql2);
pgsql2=NULL;
}
}
void kore_worker_configure(){
	kore_log(LOG_INFO, red "enum wowa: %d" rst, WOWA);
kore_log(LOG_INFO,"worker_configure\n");
atexit(han);
/*
 // it sucks, don't know how to properly  catch sigint to free some stuff
struct sigaction sa;
memset(&sa,0,sizeof(sa));
sa.sa_handler=signal_handler;
if(sigfillset(&sa.sa_mask)==-1)fatal("sigfillset: %s",errno_s);
if(sigaction(SIGINT,&sa,NULL)==-1)fatal("sigaction: %s",errno_s);
*/ 
//(void)




kore_pgsql_register(q_name,"dbname=postgres");
kore_pgsql_register("fucker","dbname=postgres");
kore_pgsql_register(q_subscribe,"dbname=postgres");
kore_pgsql_register(q_on_boss_job_fail,"dbname=postgres");
struct kore_pgsql*pgsql; 
pgsql=kore_calloc(1,sizeof(*pgsql));
kore_pgsql_init(pgsql);
kore_pgsql_bind_callback(pgsql, db_state_change, NULL);
db_query(pgsql, q_fucker,"LISTEN revents;LISTEN on_coders");
for_fucker=pgsql;

pgsql2=kore_calloc(1,sizeof(*pgsql2));
kore_pgsql_init(pgsql2);
struct boss_fail *boss=kore_calloc(1,sizeof(*boss));

boss->jobid=NULL;
kore_pgsql_bind_callback(pgsql2, db_state_change, boss);

tim=kore_timer_add(tick,5000,pgsql2,0);
u_int64_t mis=kore_time_ms();
printf("%" PRIu64 "\n",mis);
kore_log(LOG_INFO,green "ms: %" PRIu64 "" rst, mis);
}

void signal_handler(int sig){
blog_sig=sig;	
}

void tick(void*unused, u_int64_t now){
struct kore_pgsql*p=(struct kore_pgsql*)unused;
if(p==NULL)return;
kore_log(LOG_INFO,"tick-tack");
printf(yellow "%" PRIu64 "\n" rst,now);
if(blog_sig==SIGINT){kore_log(LOG_INFO,"sig int?");}

const char*limit_int="1";
struct kore_buf*b;
b=kore_buf_alloc(64);
const char*jobbi="jobbi";
// name = ANY('{jobbi,fucker}')
kore_buf_appendf(b,"{\"%s\"}",jobbi);

char *s=kore_buf_stringify(b,NULL);
kore_log(LOG_INFO, red "BECOMING: %s" rst,s);
db_query_params(p,q_subscribe,das_nextJob_query,0,2,s,strlen(s),0,limit_int,strlen(limit_int),0);
kore_free(b);
}


int init(int s){
	
kore_log(LOG_INFO,"init()");
/*
kore_pgsql_register(q_name,"dbname=postgres");
kore_pgsql_register("fucker","dbname=postgres");
//return (KORE_RESULT_OK);
struct kore_pgsql*pgsql; 

pgsql=kore_calloc(1,sizeof(*pgsql));
kore_pgsql_init(pgsql);

kore_pgsql_bind_callback(pgsql, db_state_change, NULL);
db_query(pgsql, q_fucker,"LISTEN revents;LISTEN on_coders");
//db_query(pgsql,q_name, "LISTEN revents");
//db_query(pgsql,q_name,"LISTEN on_coders");
*/ 
return (KORE_RESULT_OK);	
}


void db_state_change(struct kore_pgsql*p,void*d){
kore_log(LOG_INFO,"db_state_change: %d",p->state);

switch(p->state){
case KORE_PGSQL_STATE_INIT:
kore_log(LOG_INFO, yellow "***cb state db_init***" rst);
//db_init(c,pgsql,1);
break;
case KORE_PGSQL_STATE_WAIT:
kore_log(LOG_INFO, yellow "cb state wait" rst);
break;

case KORE_PGSQL_STATE_DONE:
kore_log(LOG_INFO,yellow "*** PGSQL_STATE_DONE!!!*** command_status: %s" rst, PQcmdStatus(p->result));
//printf(red "int extra : %d\n" rst,(int)p->arg);
printf(green "name: %s\n" rst,p->conn->name);
if(!strcmp(p->conn->name,q_fucker)){printf("str ok?\n");
	//p->state=KORE_PGSQL_STATE_COMMANDOK;
}else{
printf("string is not ok?\n");


if(!strcmp(p->conn->name,"subscribe"/*q_on_boss_job_fail*/)){
kore_log(LOG_INFO,red "ok it's a boss fail DONE" rst);
struct boss_fail*boss=(struct boss_fail*)d;
printf("aha\n");
kore_log(LOG_INFO, red "boss->jobid: %s" rst,boss->jobid);
boss_done(NULL,boss->jobid,"From c an error",NULL);
kore_free(boss->jobid);
boss->jobid=NULL;	
}



kore_log(LOG_INFO,yellow "after boss_done()" rst);

kore_pgsql_continue(p);	
}
break;
case KORE_PGSQL_STATE_COMMANDOK:
kore_log(LOG_INFO,yellow "**STATE COMMANDOK!***" rst);
printf(green "name: %s\n" rst,p->conn->name);
kore_log(LOG_INFO,yellow "command_status %s" rst, PQcmdStatus(p->result));
break;
case KORE_PGSQL_STATE_NOTIFY:
kore_log(LOG_INFO,"****notify_1111111111111111111***");
if(!strcmp(p->conn->name,q_fucker)){printf("str ok?\n");}else{printf("string is not ok?\n");}
on_notify(p);
break;

case KORE_PGSQL_STATE_COMPLETE:
kore_log(LOG_INFO, yellow "*** CB STATE COMPLETE *** %d" rst, KORE_PGSQL_STATE_COMPLETE);

break;

case KORE_PGSQL_STATE_ERROR:
kore_log(LOG_INFO, red "cb state error CB STATE ERROR %d" rst, KORE_PGSQL_STATE_ERROR);
kore_log(LOG_INFO, green "name: %s" rst, p->conn->name);
kore_log(LOG_INFO,red "read result err: %s" rst, PQerrorMessage(p->conn->db));
kore_pgsql_logerror(p);
//pg_cb_on_error(p,d);
break;
case KORE_PGSQL_STATE_RESULT:
kore_log(LOG_INFO, yellow "cb state result" rst);
kore_log(LOG_INFO, green "name: %s" rst, p->conn->name);
db_results(p,d);
break;
default:
kore_log(LOG_INFO, yellow "cb state default" rst);
kore_log(LOG_INFO, green "name: %s" rst, p->conn->name);
kore_pgsql_continue(p);
break;
}		
}


void db_query(struct kore_pgsql*p,const char*qname,const char*str_query){


if(!kore_pgsql_setup(p, qname, KORE_PGSQL_ASYNC)){
if(p->state==KORE_PGSQL_STATE_INIT){
kore_log(LOG_INFO,"waiting for available pgsql connection");
return;	
}
kore_log(LOG_INFO, red "err here" rst);
kore_pgsql_logerror(p);
return;	
}
kore_log(LOG_INFO,green "got pgsql connection" rst);
if(!kore_pgsql_query(p,str_query))
{
kore_log(LOG_INFO,red "err here2" rst);
kore_pgsql_logerror(p);
return;	
}

kore_log(LOG_INFO,yellow "query fired off!" rst);	
}

void on_notify(struct kore_pgsql*p){
kore_log(LOG_INFO,"what came?");
kore_log(LOG_INFO,"pgsql->notify.extra: %s",p->notify.extra);
kore_log(LOG_INFO,"pgsql->notify.channel: %s",p->notify.channel);
kore_websocket_broadcast(NULL,WEBSOCKET_OP_TEXT,p->notify.extra,strlen(p->notify.extra),/*WEBSOCKET_BROADCAST_GLOBAL*/4);
}

void db_results(struct kore_pgsql*p, void*data){
if(!strcmp(q_subscribe, p->conn->name)){
kore_log(LOG_INFO,green "it's subscribe result!" rst);
char *name2;int i2,rows2;char* jobid;char*jdata;
rows2=kore_pgsql_ntuples(p);
kore_log(LOG_INFO,yellow "rows2: %d" rst,rows2);
// id | name | data
//if(rows2==0)return;
if(rows2==1){
for(i2=0;i2<rows2;i2++){
name2=kore_pgsql_getvalue(p,i2,1);
jobid=kore_pgsql_getvalue(p,i2,0);
jdata=kore_pgsql_getvalue(p,i2,2);
}

kore_log(LOG_INFO,green "jobid: %s" rst,jobid);
kore_log(LOG_INFO,green "name2: %s" rst,name2);
kore_log(LOG_INFO,green "jdata: %s" rst,jdata);
struct boss_fail *boss=(struct boss_fail*)data;
//struct boss_fail boss=data;
boss->jobid=kore_strdup(jobid);
// let's make an error into flow (fake)
//1.pgsql,2.jobid string uuid,3.error string clarify if any,4.success string clarify if any
//boss_done(p,jobid,"from c an error",NULL);
}
}else if(!strcmp(q_on_boss_job_fail,p->conn->name)){
kore_log(LOG_INFO, green "it's a %s result occured!" rst, q_on_boss_job_fail);	
int rows3=kore_pgsql_ntuples(p);
kore_log(LOG_INFO,yellow "rows3: %d" rst,rows3);
}else{
struct connection*c=(struct connection*)data;
printf("entering into db_results()\n");
printf("memo2: %p\n",(void*)c->hdlr_extra);
printf("memo pgsql->arg: %p\n",(void*)p->arg);
//printf(red "db_results int extra: %d\n" rst,(int)p->addddrg);
char *name;int i,rows;char*dame=NULL;
rows=kore_pgsql_ntuples(p);

for(i=0;i<rows;i++){
name=kore_pgsql_getvalue(p,i,0);
//dame=kore_pgsql_getvalue(p,i,1);
}

printf("rows: %d\n",rows); 
kore_log(LOG_INFO,green "result name: %s" rst,name);
if(dame !=NULL)kore_log(LOG_INFO,green "result alt: %s" rst,dame);
//kore_websocket_broadcast(NULL,WEBSOCKET_OP_TEXT,name,strlen(name),/*WEBSOCKET_BROADCAST_GLOBAL*/4);	
kore_websocket_send(c,WEBSOCKET_OP_TEXT,name,strlen(name));
//kore_pgsql_continue(p);	
}
}
const char*das_boss_fail_query="WITH results AS ( \
      UPDATE pgboss.job \
      SET state = CASE \
          WHEN retryCount < retryLimit \
          THEN 'retry'::pgboss.job_state \
          ELSE 'failed'::pgboss.job_state \
          END,        \
        completedOn = CASE \
          WHEN retryCount < retryLimit \
          THEN NULL \
          ELSE now() \
          END, \
        startAfter = CASE \
          WHEN retryCount = retryLimit THEN startAfter \
          WHEN NOT retryBackoff THEN now() + retryDelay * interval '1' \
          ELSE now() + \
            ( \
                retryDelay * 2 ^ LEAST(16, retryCount + 1) / 2 \
                + \
                retryDelay * 2 ^ LEAST(16, retryCount + 1) / 2 * random() \
            ) \
            * interval '1' \
          END \
      WHERE id = ANY($1) \
        AND state < 'completed' \
      RETURNING * \
    ) \
    INSERT INTO pgboss.job (name, data) \
    SELECT \
      name || '__state__completed', \
      jsonb_build_object( \
    'request', jsonb_build_object('id', id, 'name', name, 'data', data), \
    'response', $2::jsonb, \
    'state', state, \
    'retryCount', retryCount, \
    'createdOn', createdOn, \
    'startedOn', startedOn, \
    'completedOn', completedOn, \
    'failed', CASE WHEN state = 'completed' THEN false ELSE true END \
  ) \
    FROM results \
    WHERE state = 'failed' \
      AND NOT name LIKE '%__state__completed' \
    RETURNING 1";

void boss_done(struct kore_pgsql*p,char*jobid,char*eri,char*erfolgi){
	kore_log(LOG_INFO,green "entering boss done" rst);
	if(jobid==NULL){kore_log(LOG_INFO,red "jobid is NULL! returning" rst); return;}
if(eri !=NULL){
if(erfolgi !=NULL)return;
kore_log(LOG_INFO,red "jobid in boss done: %s" rst,jobid);
//kore_log(LOG_INFO,green "memo of pgsl: %p" rst, (void*)p);


struct kore_pgsql*pgsql; 
pgsql=kore_calloc(1,sizeof(*pgsql));
kore_pgsql_init(pgsql);
kore_pgsql_bind_callback(pgsql, db_state_change, NULL);



struct kore_buf*b,*b1;
b1=kore_buf_alloc(64);
b=kore_buf_alloc(64);

//const char*jobbi="jobbi";
// id = ANY('{jobbi,fucker}')
kore_buf_appendf(b1,"{\"%s\"}", jobid);


// response,$2::jsonb
kore_buf_appendf(b,"{\"value\": \"%s\"}",eri);
char*s1=kore_buf_stringify(b1,NULL);
char *s=kore_buf_stringify(b,NULL);
kore_log(LOG_INFO, red "BECOMING: %s" rst,s);
//kore_log(LOG_INFO, red "BECOMING_s1: %s conn->name: %s" rst,s1,p->conn->name);
db_query_params(pgsql,q_on_boss_job_fail,das_boss_fail_query,0,2,s1,strlen(s1),0,s,strlen(s),0);
//kore_pgsql_continue(p);
kore_free(b1);
kore_free(b);
	
}	
}



int
page(struct http_request *req)
{
printf(yellow "page started\n" rst);
if (req->method != HTTP_METHOD_GET) {
http_response_header(req, "allow", "get");
http_response(req, 405, NULL, 0);
return (KORE_RESULT_OK);
}

struct kore_pgsql sql;
kore_pgsql_init(&sql);
if(!kore_pgsql_setup(&sql, q_name, KORE_PGSQL_SYNC)){
kore_pgsql_logerror(&sql);
}
if(!kore_pgsql_query(&sql,"update banners set alt='Amazons'")){
kore_pgsql_logerror(&sql);
}

http_response_header(req, "content-type", "text/html");
http_response(req, 200, asset_index_html, asset_len_index_html);
kore_pgsql_cleanup(&sql);
return (KORE_RESULT_OK);
}
void websocket_connect(struct connection*c){
kore_log(LOG_INFO,yellow "ws client connected. %p" rst,(void*)c);
connection_new(c);
if(c->hdlr_extra !=NULL){
db_query(c->hdlr_extra,q_name, "update banners set alt='Chicago Bulls'");
//db_query(c->hdlr_extra,q_name, "update banners set fake='Chicago Bulls'");
}
}
void websocket_disconnect(struct connection*c){
kore_log(LOG_INFO, yellow "ws client disconnected.%p" rst,(void*)c);	
connection_del(c);///???
}
void websocket_message(struct connection*c,u_int8_t op,void*data,size_t len){
kore_log(LOG_INFO, yellow "on ws message." rst);

kore_websocket_send(c,op,data,len);	
if(c->hdlr_extra !=NULL){
//db_query(c->hdlr_extra,q_name, "update banners set alt='Fuck me please!'");
//db_query(c->hdlr_extra,q_name,"update coders set name='Linux'");
//db_query(c->hdlr_extra,q_name,"update banners set alt='New World'");
//db_query(c->hdlr_extra,q_name,"update banners set alt='France';update coders set name='Feoder'");
//db_query(c->hdlr_extra,q_name,"select*from coders");
//db_query(c->hdlr_extra,q_name,"select*from coders;select*from coders");
printf("memo1: %p\n",(void*)c->hdlr_extra);
//((struct pgsql*)c->hdlr_extra)->arg="s";
//db_query(c->hdlr_extra,q_name,"select*from coders;select*from banners");
db_query(c->hdlr_extra,q_name,"select*from coders;update banners set alt='fucky'");
}
}
int page_ws_connect(struct http_request*req){
kore_websocket_handshake(req,"websocket_connect","websocket_message","websocket_disconnect");
return (KORE_RESULT_OK);	
}
void connection_new(struct connection*c){
struct kore_pgsql*pgsql;
c->disconnect=connection_del;
//c->disconnect=NULL;
//c->proto=CONN_PROTO_UNKNOWN;
c->state=CONN_STATE_ESTABLISHED;
pgsql=kore_calloc(1,sizeof(*pgsql));
kore_pgsql_init(pgsql);
kore_pgsql_bind_callback(pgsql, db_state_change, c);
c->hdlr_extra=pgsql;
kore_log(LOG_INFO, yellow "new calback db connection: %p" rst,(void*)c);
}
void connection_del(struct connection*c){
kore_log(LOG_INFO, yellow "connection db cb disconnecting..: %p" rst,(void*)c);
if(c->hdlr_extra !=NULL)kore_pgsql_cleanup(c->hdlr_extra);
kore_free(c->hdlr_extra);
c->hdlr_extra=NULL;
}


void db_query_params(struct kore_pgsql*p,const char*qname,const char*str_query,int result,int counts,...){
if(!kore_pgsql_setup(p, qname, KORE_PGSQL_ASYNC)){
if(p->state==KORE_PGSQL_STATE_INIT){
kore_log(LOG_INFO,"waiting for available pgsql connection");
return;	
}
kore_log(LOG_INFO, red "err here" rst);
kore_pgsql_logerror(p);
return;	
}
int ret;
va_list		args;
va_start(args, counts);
ret = kore_pgsql_v_query_params(p, str_query, result, counts, args);
va_end(args);
if(ret !=1){kore_pgsql_logerror(p);return;}
kore_log(LOG_INFO,red "***ret*** va_list : %d" rst,ret);// 1 is OK
}
