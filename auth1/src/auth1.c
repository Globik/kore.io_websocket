#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>
#include "assets.h"

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"

char *sess=NULL;

int		page(struct http_request *);
int		login(struct http_request*);
int dashboard(struct http_request*);
int	redirect(struct http_request *);
int auth_logout(struct http_request*);

int	auth_login(struct http_request *);
int	auth_user_exists(struct http_request *, char *);
//void	auth_session_add(struct kore_msg *, const void *);
//void	auth_session_del(struct kore_msg *, const void *);
int	auth_session(struct http_request *, const char *);
int init(int state);

#define REQ_STATE_INIT 0
#define REQ_STATE_QUERY 1
#define REQ_STATE_DB_WAIT 2
#define REQ_STATE_DB_READ 3
#define REQ_STATE_ERROR 4
#define REQ_STATE_DONE 5

const char*dbc="db";
const char*sessi="sess_auth";

//void on_notify(struct kore_pgsql*);
void db_state_change(struct kore_pgsql*,void*);
void db_query(struct kore_pgsql*,const char*, const char*);
void db_query_params(struct kore_pgsql*p,const char*qname,const char*str_query);
void db_results(struct kore_pgsql*,void*);


static int request_perform_init(struct http_request*);
static int request_perform_query(struct http_request*);
static int request_db_wait(struct http_request*);
static int request_db_read(struct http_request*);
static int request_error(struct http_request*);
static int request_done(struct http_request*);

struct http_state mystates[]={
{"REQ_STATE_INIT",request_perform_init},
{"REQ_STATE_QUERY",request_perform_query},
{"REQ_STATE_DB_WAIT",request_db_wait},
{"REQ_STATE_DB_READ",request_db_read},
{"REQ_STATE_ERROR",request_error},
{"REQ_STATE_DONE",request_done},
};

#define mystates_size (sizeof(mystates)/sizeof(mystates[0]))

struct rstate{
	int cnt;
	char*query;
	char*q_name;
	//char*us_sessi;
	struct kore_pgsql sql;
};
struct ustate{
char*result_name;
union{int a;int b;}mu;
struct kore_pgsql p;	
};

int init(int state){
kore_pgsql_register(dbc, "dbname=postgres");
kore_pgsql_register(sessi,"dbname=postgres");
return (KORE_RESULT_OK);	
}

int page(struct http_request *req)
{
kore_log(LOG_INFO,yellow "The page started. Path: %s" rst,req->path);
if (req->method != HTTP_METHOD_GET) {
kore_log(LOG_INFO,red "method not allowed?" rst);
http_response_header(req, "allow", "get");
http_response(req, 405, NULL, 0);
return (KORE_RESULT_OK);
}
//http_response(req, 200, NULL, 0);
//http_response_header(req, "content-type", "text/html");
//http_response(req, 200, asset_page_html, asset_len_page_html);
//return (KORE_RESULT_OK);
struct rstate *state;
if(!http_state_exists(req)){
state=http_state_create(req,sizeof(*state));
state->cnt=12;
//state->query=kore_strdup("update coders set name='feoder'");
state->query=kore_strdup("select alt from banners where alt='user_sess'");
state->q_name=kore_strdup(dbc);
//state->us_sessi=NULL;
kore_pgsql_init(&state->sql);
kore_pgsql_bind_request(&state->sql,req);	
}
return (http_state_run(mystates,mystates_size,req));
}

int request_perform_init(struct http_request*req){
	printf("PERFORM INIT\n");
struct rstate *state;
//if(!http_state_exists(req)){
//state=http_state_create(req,sizeof(*state));
//kore_pgsql_init(&state->sql);
//kore_pgsql_bind_request(&state->sql,req);	
//}else{
state=http_state_get(req);
kore_log(LOG_INFO,green "12? state->cnt: %d" rst,state->cnt);
kore_log(LOG_INFO,green "a query?: %s" rst,state->query);
kore_log(LOG_INFO,green "state->q_name: %s" rst,state->q_name);
//}
if(!kore_pgsql_setup(&state->sql,state->q_name,KORE_PGSQL_ASYNC)){
kore_log(LOG_INFO,red "no setup" rst);
if(state->sql.state==KORE_PGSQL_STATE_INIT){
kore_log(LOG_INFO,red "kore pgsql state init" rst);
req->fsm_state=REQ_STATE_INIT;
return (HTTP_STATE_RETRY);
}	
kore_pgsql_logerror(&state->sql);
req->fsm_state=REQ_STATE_ERROR;
}else{req->fsm_state=REQ_STATE_QUERY;kore_log(LOG_INFO,red "req_state_query" rst);}
return (HTTP_STATE_CONTINUE);
}
int request_perform_query(struct http_request*req){
struct rstate*state=http_state_get(req);
req->fsm_state=REQ_STATE_DB_WAIT;
//if(!kore_pgsql_query(&state->sql,"select*from coders"))
//if(!kore_pgsql_query(&state->sql,"update coders set name='Linux'"))
if(!kore_pgsql_query(&state->sql,state->query))
//if(!kore_pgsql_query(&state->sql,"update coders set name='Linux';update banners set alt='Fucker'"))
//if(!kore_pgsql_query(&state->sql,"update coders set fake='Linux'"))
//if(!kore_pgsql_query(&state->sql,"update coders set fake='Linux';update banners set fake='Drubich'"))
{
kore_log(LOG_INFO,yellow "no query" rst);
return (HTTP_STATE_CONTINUE);	
}
kore_log(LOG_INFO,green "query fiered off?" rst);
return (HTTP_STATE_RETRY);	
}

int request_db_wait(struct http_request*req){
struct rstate*state=http_state_get(req);
kore_log(LOG_INFO,red "request db wait: %d" rst,state->sql.state);
switch(state->sql.state){
case KORE_PGSQL_STATE_WAIT:
kore_log(LOG_INFO,"state wait");
return (HTTP_STATE_RETRY);
case KORE_PGSQL_STATE_COMPLETE:
kore_log(LOG_INFO,"state complete");
req->fsm_state=REQ_STATE_DONE;
break;
case KORE_PGSQL_STATE_COMMANDOK:
kore_log(LOG_INFO,red "state command ok" rst);
kore_log(LOG_INFO,yellow "command on command ok status: %s\n" rst, PQcmdStatus((&state->sql)->result));	
kore_pgsql_continue(&state->sql);
break;
case KORE_PGSQL_STATE_ERROR:
kore_log(LOG_INFO,"state error");
req->fsm_state=REQ_STATE_ERROR;
kore_pgsql_logerror(&state->sql);
break;
case KORE_PGSQL_STATE_RESULT:
kore_log(LOG_INFO,red "state_result" rst);
req->fsm_state=REQ_STATE_DB_READ;
break;
default:
kore_log(LOG_INFO,red "defualt" rst);
kore_pgsql_continue(&state->sql);
}
return (HTTP_STATE_CONTINUE);	
}

int request_db_read(struct http_request*req){
	kore_log(LOG_INFO,green "request_db_read()" rst);
char*name;
int i,rows;
struct rstate*state=http_state_get(req);
rows=kore_pgsql_ntuples(&state->sql);
for(i=0;i<rows;i++){
name=kore_pgsql_getvalue(&state->sql,i,0);
kore_log(LOG_INFO,"name: %s",name);	
}
kore_pgsql_continue(&state->sql);
req->fsm_state=REQ_STATE_DB_WAIT;
return (HTTP_STATE_CONTINUE);	
}

int request_error(struct http_request*req){
kore_log(LOG_INFO,red "request_error()" rst);	
struct rstate*state=http_state_get(req);
kore_log(LOG_INFO,red "error?: %s" rst,(&state->sql)->error);

http_response(req,500,(&state->sql)->error,strlen((&state->sql)->error));
kore_free(state->query);
kore_free(state->q_name);
kore_pgsql_cleanup(&state->sql);
http_state_cleanup(req);

return (HTTP_STATE_COMPLETE);	
}
int request_done(struct http_request*req){
kore_log(LOG_INFO,green "REQUEST_DONE()???***" rst);
struct rstate*state=http_state_get(req);
//if(state->us_sessi !=NULL)kore_log(LOG_INFO,green "state->us_sessi: %s" rst,state->us_sessi);
kore_free(state->query);
kore_free(state->q_name);
kore_pgsql_cleanup(&state->sql);
http_state_cleanup(req);
//http_response(req, 200, NULL, 0);
http_response_header(req, "content-type", "text/html");
http_response(req, 200, asset_page_html, asset_len_page_html);
return (HTTP_STATE_COMPLETE);
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
//if(!strcmp(p->conn->name,q_fucker)){printf("str ok?\n");}else{
//printf("string is not ok?\n");
kore_pgsql_continue(p);	
//}
break;
case KORE_PGSQL_STATE_COMMANDOK:
kore_log(LOG_INFO,yellow "COMMANDOK!" rst);
kore_log(LOG_INFO,yellow "command_status %s" rst, PQcmdStatus(p->result));
break;
case KORE_PGSQL_STATE_NOTIFY:
kore_log(LOG_INFO,"****notify_1111111111111111111***");
//if(!strcmp(p->conn->name,q_fucker)){printf("str ok?\n");}else{printf("string is not ok?\n");}
//on_notify(p);
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

void db_query_params(struct kore_pgsql*p,const char*qname,const char*str_query){


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
char*coder="user_sess";
/*
int person_id=htonl(101);
const char*values[2]={(char*)&person_id, coder};
int lengths[2]={sizeof(person_id), strlen(coder)};
int formats[2]={1, 0};
* 2, NULL, values, lengths, formats, 0
* number of params,
* ignore the oid field,
* values to substitute $1 and $2,
* the lengths, in bytes, of each of param values;
* wether the values are binary or not
* we want the result in text format
*/ 
// http://timmurphy.org/2009/11/19/pgexecparams-example-postgresql-query-execution-with-parameters/
// select alt from banners where alt='user_sess'
//int kore_pgsql_query_params(struct kore_pgsql *pgsql,const char *query, int result, int count, ...)
// Joris: kore_pgsql_query_params(sql,"select foo from disco_bar where something=$1",result_format,1,value,strlen(value),value_format)
/* where result_format = 0 for text or 1 for binary, value_format = 0 for text or 1 for binary
 * The arguments given to the kore function are basically a set of 3 per argument for the query :
 * value, length of value, type of value.
 */
if(!kore_pgsql_query_params(p,"select alt from banners where alt=$1",0,1,coder,strlen(coder),0))
{
kore_log(LOG_INFO,red "err here2" rst);
kore_pgsql_logerror(p);
return;	
}
kore_log(LOG_INFO,yellow "query fired off!" rst);	
}



void db_results(struct kore_pgsql*p,void*c){
kore_log(LOG_INFO,green "Entering into db_results(),p->conn->name: %s" rst,p->conn->name);

//printf("memo2: %p\n",(void*)c->hdlr_extra);
//printf("memo pgsql->arg: %p\n",(void*)p->arg);
//printf(red "db_results int extra: %d\n" rst,(int)p->arg);
if(!strcmp(sessi, p->conn->name)){
struct http_request*req=(struct http_request*)c;
if(req==NULL){kore_log(LOG_INFO,red "req is NULL in db results cb" rst);}
char *name;int i,rows;
//char*dame=NULL;
rows=kore_pgsql_ntuples(p);
if(rows==0)return;
for(i=0;i<rows;i++){
name=kore_pgsql_getvalue(p,i,0);
//dame=kore_pgsql_getvalue(p,i,1);
}

kore_log(LOG_INFO,green "result name: %s" rst,name);
struct ustate*state;
state=http_state_get(req);
if(state->result_name==NULL){
kore_log(LOG_INFO,yellow "Aha, state->result_name is NULL" rst);
state->result_name=kore_strdup(name);
state->mu.a=3;state->mu.b=4;
}
}
}








int login(struct http_request *req)
{
kore_log(LOG_INFO,yellow "The login started. Path: %s" rst,req->path);
if (req->method != HTTP_METHOD_GET) {
kore_log(LOG_INFO,red "method not allowed?" rst);
http_response_header(req, "allow", "get");
http_response(req, 405, NULL, 0);
return (KORE_RESULT_OK);
}
char*value;

http_populate_cookies(req);

if(http_request_cookie(req,"hicookie",&value)){
kore_log(LOG_INFO,red "hicookie: %s" rst, value);
}else{kore_log(LOG_INFO,red "no hicookie" rst);}

//http_response(req, 200, NULL, 0);
http_response_header(req, "content-type", "text/html");
http_response(req, 200, asset_login_html, asset_len_login_html);
return (KORE_RESULT_OK);
}

int auth_login(struct http_request *req)
{
kore_log(LOG_INFO,yellow "auth_login: The %s started." rst,req->path);
char*vuser;
char*pass;
struct http_cookie	*cookie;
if (req->method != HTTP_METHOD_POST) {
http_response_header(req, "allow", "post");
http_response(req, 405, "po", 2);
return (KORE_RESULT_OK);
}

if(req->method==HTTP_METHOD_POST){
http_populate_post(req);

if(!http_argument_get_string(req,"user", &vuser)){
	// || !http_argument_get_string(req,"passphrase",&pass)){
kore_log(LOG_INFO, red "user is NOT provided." rst);
//req->method=HTTP_METHOD_GET;
//return (asset_serve_login_html(req));
//http_response_header(req, "content-type", "text/html");
//http_response(req, 200, asset_login_html, asset_len_login_html);
http_response(req,200,"nouser",6);
return (KORE_RESULT_OK);	
}
 
if(!http_argument_get_string(req,"passphrase",&pass)){
kore_log(LOG_INFO,red "No passphrase is NOT provided!" rst);
http_response(req,200,"nopwd",5);
return (KORE_RESULT_OK);
}
kore_log(LOG_INFO,green "user: " yellow "%s" rst,vuser);
kore_log(LOG_INFO, green "The password: %s" rst,pass);
	
if(!strcmp(vuser,"globi")){}else{
kore_log(LOG_INFO,red "No such user=> %s" rst,vuser);
req->method=HTTP_METHOD_GET;
//return (asset_serve_login_html(req));
http_response_header(req, "content-type", "text/html");
http_response(req, 200, asset_login_html, asset_len_login_html);
return (KORE_RESULT_OK);	
}
if(!strcmp("globi",pass)){}else{
kore_log(LOG_INFO,red "Invalid password: %s" rst, pass);
req->method=HTTP_METHOD_GET;
//return (asset_serve_login_html(req));	
http_response_header(req, "content-type", "text/html");
http_response(req, 200, asset_login_html, asset_len_login_html);
return (KORE_RESULT_OK);
}

}
//http_response(req, 200, "ok", 2);
char*value;

http_populate_cookies(req);

if(http_request_cookie(req,"hicookie",&value)){
kore_log(LOG_INFO,red "hicookie: %s" rst, value);
}else{kore_log(LOG_INFO,red "no hicookie" rst);}


if(sess==NULL){kore_log(LOG_INFO,"adding a session");sess="user_sess";}
http_response_header(req,"location","/dashboard/");
http_response_cookie(req,"hicookie","user_sess","/",0,0,&cookie);
cookie->flags &= ~HTTP_COOKIE_HTTPONLY;
cookie->flags &= ~HTTP_COOKIE_SECURE;
kore_log(LOG_INFO,green "user session: %s" rst,sess);
http_response(req, HTTP_STATUS_FOUND,"NULL",4);
//req->method=HTTP_METHOD_GET;
//http_response(req,200,asset_login_html, asset_len_login_html);

 //http_response_header(req, "location", "/drafts/");
//	http_response_cookie(req, "blog_token", session.data,    "/drafts/", 0, 0, NULL);

	//kore_log(LOG_INFO, "login for '%s'", up->name);
	//http_response(req, HTTP_STATUS_FOUND, NULL, 0);

return (KORE_RESULT_OK);
}

int dashboard(struct http_request *req)
{
kore_log(LOG_INFO,yellow "The %s started." rst,req->path);
struct ustate *state;
if (req->method != HTTP_METHOD_GET) {
http_response_header(req, "allow", "get");
http_response(req, 405, NULL, 0);
return (KORE_RESULT_OK);
}
//http_response(req, 200, NULL, 0);
//if(req->hdlr_extra !=NULL){kore_log(LOG_INFO,green "any data: %s" rst,(char*)req->hdlr_extra);req->hdlr_extra=NULL;}
if(http_state_exists(req)){
state=http_state_get(req);	
if(state->result_name && state->result_name !=NULL){
kore_log(LOG_INFO,yellow "result name: %s" rst,state->result_name);
kore_log(LOG_INFO,red "union int a: %d : b: %d" rst,state->mu.a,state->mu.b);
kore_free(state->result_name);
	
}
//kore_pgsql_cleanup(&state->p);
http_state_cleanup(req);
}
http_response_header(req, "content-type", "text/html");
http_response(req, 200, asset_dashboard_html, asset_len_dashboard_html);
return (KORE_RESULT_OK);
}

int auth_logout(struct http_request *req)
{
kore_log(LOG_INFO,yellow "The %s started." rst,req->path);
if (req->method != HTTP_METHOD_GET) {
http_response_header(req, "allow", "get");
http_response(req, 405, NULL, 0);
return (KORE_RESULT_OK);
}
sess=NULL;
http_response_header(req,"set-cookie","hicookie=null; path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT");//?
//http_response(req, 200, "ok_logout", 9);
redirect(req);
return (KORE_RESULT_OK);
}


int auth_session(struct http_request*req, const char*cookie){
kore_log(LOG_INFO,"Entering auth_session().");
if(cookie==NULL){
kore_log(LOG_INFO,red "cookie is NULL. Return." rst);	
return (KORE_RESULT_ERROR);
}	
kore_log(LOG_INFO,"cookie: %s, path: %s", cookie,req->path);
/*
if(!strcmp("user_sess", cookie)){
kore_log(LOG_INFO,green "Cookie compare is OK" rst);
if(req->hdlr_extra==NULL){req->hdlr_extra="ABBA";}
return (KORE_RESULT_OK);	
}
*/
struct ustate *state;
if(!http_state_exists(req)){
state=http_state_create(req,sizeof(*state));
state->result_name=NULL;
kore_pgsql_init(&state->p);
kore_pgsql_bind_callback(&state->p,db_state_change,req);
db_query_params(&state->p,sessi,"select alt from banners where alt='user_sess'");
return (KORE_RESULT_RETRY);
}else{
kore_log(LOG_INFO,red "extra is NOT NULL***" rst);
state=http_state_get(req);
}	
kore_log(LOG_INFO,red "STATE***: %d" rst, (&state->p)->state);

if((&state->p)->state==KORE_PGSQL_STATE_COMPLETE){
kore_log(LOG_INFO,red "das ist result***!!!" rst);
http_request_wakeup(req);
if(state->result_name !=NULL){
kore_log(LOG_INFO,red "STATE->RESULT_NAME: %s" rst,state->result_name);	
if(!strcmp(state->result_name,cookie)){
kore_log(LOG_INFO,green "cookie compare is OK" rst);
return (KORE_RESULT_OK);
}else{
kore_log(LOG_INFO,red "cookie comapare is NOT OK" rst);
kore_free(state->result_name);
state->result_name=NULL;
}
}
kore_pgsql_cleanup(&state->p);
http_state_cleanup(req);

return (KORE_RESULT_ERROR);
}else if((&state->p)->state==KORE_PGSQL_STATE_ERROR){
	// ??? it's not reached here, one need one more field for db error report after state complete
kore_log(LOG_INFO, red "some err in db" rst);
http_request_wakeup(req);

kore_pgsql_cleanup(&state->p);

http_state_cleanup(req);
return (KORE_RESULT_ERROR);	
}else{
kore_log(LOG_INFO,yellow "*** schon wieder retry???***" rst);
http_request_sleep(req);
return (KORE_RESULT_RETRY);
}
return (KORE_RESULT_ERROR);
}


int redirect(struct http_request *req)
{
kore_log(LOG_INFO,"Redirecting...");
http_response_header(req, "location", "/");
http_response(req, HTTP_STATUS_FOUND, NULL, 0);
return (KORE_RESULT_OK);
}
int auth_user_exists(struct http_request*req,char*user){
kore_log(LOG_INFO,"Entering auth_user_exists().");
if(user==NULL){
kore_log(LOG_INFO,red "User is NULL. Return" rst);
return (KORE_RESULT_ERROR);	
}
kore_log(LOG_INFO,"User: %s",user);	
if(!strcmp("globi",user)){
kore_log(LOG_INFO,green "The user %s is exists in db!" rst,user);
return (KORE_RESULT_OK);
}
kore_log(LOG_INFO, red "No, this user: %s does NOT exist!" rst,user);
return (KORE_RESULT_ERROR);
}

