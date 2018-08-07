#include <kore/kore.h>
#include <kore/http.h>
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
http_response_header(req, "content-type", "text/html");
http_response(req, 200, asset_page_html, asset_len_page_html);
return (KORE_RESULT_OK);
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
http_response(req, HTTP_STATUS_FOUND,NULL,0);
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
if (req->method != HTTP_METHOD_GET) {
http_response_header(req, "allow", "get");
http_response(req, 405, NULL, 0);
return (KORE_RESULT_OK);
}
//http_response(req, 200, NULL, 0);
if(req->hdlr_extra !=NULL){kore_log(LOG_INFO,green "any data: %s" rst,(char*)req->hdlr_extra);req->hdlr_extra=NULL;}
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
//http_response_cookie(req,NULL,NULL,NULL,0,0,NULL);
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
if(!strcmp("user_sess", cookie)){
kore_log(LOG_INFO,green "Cookie compare is OK" rst);
if(req->hdlr_extra==NULL){req->hdlr_extra="ABBA";}
return (KORE_RESULT_OK);	
}
kore_log(LOG_INFO,red "Cookie compare is NOT OK" rst);
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

