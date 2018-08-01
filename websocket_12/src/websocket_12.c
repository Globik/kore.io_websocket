#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>
#include <kore/tasks.h>
#include "assets.h"
#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"


int	page(struct http_request *);
void kore_worker_configure(void);
int init(int);
void on_notify(struct kore_pgsql*);

void db_state_change(struct kore_pgsql*,void*);
void db_query(struct kore_pgsql*,const char*);
void db_results(struct kore_pgsql*);

void db_state_change2(struct kore_pgsql*,void*);
void db_query2(struct kore_pgsql*,const char*);
void db_query3(struct kore_pgsql*,const char*);
void db_results2(struct kore_pgsql*);


int page_ws_connect(struct http_request*);
void websocket_connect(struct connection*);
void websocket_disconnect(struct connection*);
void websocket_message(struct connection*,u_int8_t,void*,size_t);

//struct kore_pgsql*pgsql=NULL;
struct kore_pgsql*pgsql2=NULL;

int init(int s){
	kore_log(LOG_INFO,red "init()" rst);
	//kore_debug("***************DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD*************");
kore_log(LOG_INFO,red "init()" rst);

kore_pgsql_register("db","dbname=postgres");
kore_pgsql_register("dba","dbname=postgres");

struct kore_pgsql*pgsql=kore_calloc(1,sizeof(*pgsql));
kore_pgsql_init(pgsql);
int a=1;
kore_pgsql_bind_callback(pgsql, db_state_change,(void*)a);
db_query(pgsql,"LISTEN revents;LISTEN on_coders");
//db_query(pgsql,"LISTEN revents");
//db_query3(pgsql,"LISTEN on_coders");


//struct kore_pgsql*pgsql3=kore_calloc(1,sizeof(*pgsql3));
//kore_pgsql_init(pgsql3);

//kore_pgsql_bind_callback(pgsql3, db_state_change,(void*)a);
//db_query(pgsql,"LISTEN revents;LISTEN on_coders");
//db_query(pgsql3,"LISTEN on_coders");

pgsql2=kore_calloc(1,sizeof(*pgsql2));
kore_pgsql_init(pgsql2);
int b=2;
kore_pgsql_bind_callback(pgsql2,db_state_change2,(void*)b);
return (KORE_RESULT_OK);	
}
int ok=0;
void kore_worker_configure(){
kore_log(LOG_INFO,"kore_worker_configure()");
}
void db_state_change(struct kore_pgsql*p,void*d){
kore_log(LOG_INFO,"db_state_change: %d int %d",p->state,(int)d);

switch(p->state){
case KORE_PGSQL_STATE_INIT:
kore_log(LOG_INFO, yellow "***cb state db_init***" rst);
//db_init(c,pgsql,1);
break;
case KORE_PGSQL_STATE_WAIT:
kore_log(LOG_INFO, yellow "cb state wait" rst);
//kore_pgsql_continue(p);
break;

case KORE_PGSQL_STATE_DONE:
kore_log(LOG_INFO,yellow "command_status %s" rst,PQcmdStatus(p->result));
kore_debug("SUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUKA!\n");
//kore_debug_internal(__FILE__, __LINE__,"ss","%dGGGGGGGGGGGGGGGGGG");
printf(red "int extra : %d\n" rst,(int)p->arg);
printf(green "name: %s\n" rst,p->conn->name);
printf("ok: %d\n",ok);
//if(ok==0)db_query(pgsql,"listen on_coders");
ok++;
kore_pgsql_continue(p);	
break;

case KORE_PGSQL_STATE_NOTIFY:
kore_log(LOG_INFO,"notify");
on_notify(p);
//kore_pgsql_continue(p);
break;

case KORE_PGSQL_STATE_COMPLETE:
kore_log(LOG_INFO, yellow "cb state complete" rst);
printf(red "CCCCCCCCCCCComplete int extra????: %d\n" rst,(int)p->arg);
//kore_debug("******************************************************************************8DEBUG!\n");
//kore_pgsql_continue(p);
// update banners set alt='dus';
break;

case KORE_PGSQL_STATE_ERROR:
kore_log(LOG_INFO, red "cb state error" rst);
kore_log(LOG_INFO,red "read result err: %s" rst, PQerrorMessage(p->conn->db));
kore_pgsql_logerror(p);
break;
case KORE_PGSQL_STATE_RESULT:
kore_log(LOG_INFO, yellow "cb state result" rst);
//db_results(p);
break;
default:
kore_log(LOG_INFO, yellow "cb state default" rst);
kore_pgsql_continue(p);
break;
}
//kore_pgsql_continue(p);		
}

void db_state_change2(struct kore_pgsql*p,void*d){
kore_log(LOG_INFO,"db_state_change_2: %d int %d",p->state,(int)d);

switch(p->state){
case KORE_PGSQL_STATE_INIT:
kore_log(LOG_INFO, yellow "***cb state db_init_2***" rst);
//db_init(c,pgsql,1);
break;
case KORE_PGSQL_STATE_WAIT:
kore_log(LOG_INFO, yellow "cb state wait_2" rst);

break;

case KORE_PGSQL_STATE_DONE:
kore_log(LOG_INFO,yellow "command_status_2 %s" rst,PQcmdStatus(p->result));
printf(red "int extra_2 : %d\n" rst,(int)p->arg);

kore_pgsql_continue(p);	
break;

case KORE_PGSQL_STATE_NOTIFY:
kore_log(LOG_INFO,"**notify_2**");
on_notify(p);
break;

case KORE_PGSQL_STATE_COMPLETE:
kore_log(LOG_INFO, yellow "cb state complete_2" rst);
//printf(red "complete int extra_2: %d\n" rst,(int)p->arg);

// update banners set alt='dus';
break;

case KORE_PGSQL_STATE_ERROR:
kore_log(LOG_INFO, red "cb state error_2" rst);
kore_log(LOG_INFO,red "read result err_2: %s" rst, PQerrorMessage(p->conn->db));
kore_pgsql_logerror(p);
break;
case KORE_PGSQL_STATE_RESULT:
kore_log(LOG_INFO, yellow "cb state result_2" rst);
db_results2(p);
break;
default:
kore_log(LOG_INFO, yellow "cb state default_2" rst);
kore_pgsql_continue(p);
break;
}		
}


void db_query(struct kore_pgsql*p,const char*str_query){


if(!kore_pgsql_setup(p,"db",KORE_PGSQL_ASYNC)){
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
void db_query2(struct kore_pgsql*p,const char*str_query){


if(!kore_pgsql_setup(p,"db",KORE_PGSQL_ASYNC)){
if(p->state==KORE_PGSQL_STATE_INIT){
kore_log(LOG_INFO,"waiting for available pgsql connection_2");
return;	
}
kore_log(LOG_INFO, red "err here_2" rst);
kore_pgsql_logerror(p);
return;	
}
kore_log(LOG_INFO,green "got pgsql connection_2" rst);
if(!kore_pgsql_query(p,str_query))
{
kore_log(LOG_INFO,red "err here2" rst);
kore_pgsql_logerror(p);
return;	
}
printf("query fired off_2.\n");
}

void db_query3(struct kore_pgsql*p,const char*str_query){


if(!kore_pgsql_setup(p,"dba",KORE_PGSQL_ASYNC)){
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
	//kore_pgsql_cleanup(pgsql);
	kore_websocket_broadcast(NULL,WEBSOCKET_OP_TEXT,p->notify.extra,strlen(p->notify.extra),/*WEBSOCKET_BROADCAST_GLOBAL*/4);	
	//return;
}

void db_results(struct kore_pgsql*p){

printf(red "db_results int extra: %d\n" rst,(int)p->arg);
char *name;int i,rows;
rows=kore_pgsql_ntuples(p);
for(i=0;i<rows;i++){
name=kore_pgsql_getvalue(p,i,0);
}
kore_log(LOG_INFO,green "result name: %s" rst,name);
kore_websocket_broadcast(NULL,WEBSOCKET_OP_TEXT,name,strlen(name),/*WEBSOCKET_BROADCAST_GLOBAL*/4);	
kore_pgsql_continue(p);	
}


void db_results2(struct kore_pgsql*p){

printf(red "db_results int extra_2: %d\n" rst,(int)p->arg);

char *name;int i,rows;
rows=kore_pgsql_ntuples(p);
for(i=0;i<rows;i++){
name=kore_pgsql_getvalue(p,i,0);
}
kore_log(LOG_INFO,green "result name: %s" rst,name);
//int webi=WEBSOCKET_BROADCAST_GLOBAL;
kore_websocket_broadcast(NULL,WEBSOCKET_OP_TEXT,name,strlen(name),4);	

kore_pgsql_continue(p);	
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
if(!kore_pgsql_setup(&sql,"db",KORE_PGSQL_SYNC)){
printf(red "setup err\n" rst);
kore_pgsql_logerror(&sql);
}
if(!kore_pgsql_query(&sql,"update banners set alt='Amazons'")){
printf(red "query err\n" rst);
kore_pgsql_logerror(&sql);
}

http_response_header(req, "content-type", "text/html");
http_response(req, 200, asset_index_html, asset_len_index_html);
kore_pgsql_cleanup(&sql);
return (KORE_RESULT_OK);
}
void websocket_connect(struct connection*c){
kore_log(LOG_INFO,yellow "ws client connected. %p" rst,(void*)c);


if(pgsql2 !=NULL){
printf(red "extra int? %d\n" rst,(int)pgsql2->arg);
pgsql2->arg=(void*)889;
db_query2(pgsql2,"update banners set alt='Chikago'");
}
}
void websocket_disconnect(struct connection*c){
kore_log(LOG_INFO, yellow "ws client disconnected.%p" rst,(void*)c);	
}
void websocket_message(struct connection*c,u_int8_t op,void*data,size_t len){
kore_log(LOG_INFO, yellow "on ws message." rst);

if(pgsql2 !=NULL){
pgsql2->arg=(void*)888;
//db_query2(pgsql2,"select*from banners");
db_query2(pgsql2,"update banners set alt='Afrika'");
}

kore_websocket_send(c,op,data,len);	
}
int page_ws_connect(struct http_request*req){
kore_websocket_handshake(req,"websocket_connect","websocket_message","websocket_disconnect");
return (KORE_RESULT_OK);	
}
