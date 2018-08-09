#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>
#include <kore/tasks.h>
#include "assets.h"
#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
const char*q_name="db";
const char*q_fucker="fucker";
struct kore_pgsql*for_fucker=NULL;

int	page(struct http_request *);
//void kore_worker_configure(void);
int init(int);

void on_notify(struct kore_pgsql*);

void db_state_change(struct kore_pgsql*,void*);

void db_query(struct kore_pgsql*,const char*, const char*);
void db_results(struct kore_pgsql*,struct connection*);


int page_ws_connect(struct http_request*);
void websocket_connect(struct connection*);
void websocket_disconnect(struct connection*);
void websocket_message(struct connection*,u_int8_t,void*,size_t);
void han(void);
void connection_new(struct connection*);
void connection_del(struct connection*);
void han(){
	printf(green "at exit occured.\n" rst);
	//if(for_fucker !=NULL)free(for_fucker);//kore_pgsql_cleanup(for_fucker);
	//kore_pgsql_continue(for_fucker);	
	}
void kore_worker_configure(){
	printf("configure worker\n");
	atexit(han);
	}
int init(int s){
	
kore_log(LOG_INFO,"init()");
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
kore_log(LOG_INFO,yellow "command_status %s" rst, PQcmdStatus(p->result));
//printf(red "int extra : %d\n" rst,(int)p->arg);
printf(green "name: %s\n" rst,p->conn->name);
if(!strcmp(p->conn->name,q_fucker)){printf("str ok?\n");}else{
printf("string is not ok?\n");
kore_pgsql_continue(p);	
}
break;
case KORE_PGSQL_STATE_COMMANDOK:
kore_log(LOG_INFO,yellow "COMMANDOK!" rst);
kore_log(LOG_INFO,yellow "command_status %s" rst, PQcmdStatus(p->result));
break;
case KORE_PGSQL_STATE_NOTIFY:
kore_log(LOG_INFO,"****notify_1111111111111111111***");
if(!strcmp(p->conn->name,q_fucker)){printf("str ok?\n");}else{printf("string is not ok?\n");}
on_notify(p);
break;

case KORE_PGSQL_STATE_COMPLETE:
kore_log(LOG_INFO, yellow "cb state complete" rst);
//printf(red "complete int extra: %d\n" rst,(int)p->arg);
break;

case KORE_PGSQL_STATE_ERROR:
kore_log(LOG_INFO, red "cb state error" rst);
kore_log(LOG_INFO,red "read result err: %s" rst, PQerrorMessage(p->conn->db));
kore_pgsql_logerror(p);
break;
case KORE_PGSQL_STATE_RESULT:
kore_log(LOG_INFO, yellow "cb state result" rst);
db_results(p,d);
break;
default:
kore_log(LOG_INFO, yellow "cb state default" rst);
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

void db_results(struct kore_pgsql*p,struct connection*c){
printf("entering into db_results()\n");
printf("memo2: %p\n",(void*)c->hdlr_extra);
printf("memo pgsql->arg: %p\n",(void*)p->arg);
//printf(red "db_results int extra: %d\n" rst,(int)p->arg);
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
