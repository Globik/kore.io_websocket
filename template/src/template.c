#include <kore/kore.h>
#include <kore/http.h>
#include <stdbool.h>

//const 
char*buser=NULL;
bool is_admin=false;
//int is_admin=1;

struct obj_html_head{
char*metatag;
char*title;	
};
struct obj_html_footer{
char*footer_str;
};
struct obj_html_main_menu{
bool is_admin;
char * buser;	
};

int		page(struct http_request *);
struct kore_buf*rend(char*,char*);

inline char*html_head(struct obj_html_head);
inline char*html_footer(struct obj_html_footer);
inline char*html_main_menu(struct obj_html_main_menu);

char*str_meta_str(void);

char*html_head(struct obj_html_head n){
struct kore_buf*b;
b=kore_buf_alloc(128);
kore_buf_appendf(b,"<meta charset=\"utf-8\">\n<title>%s</title>\n\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
\n<meta name=\"apple-mobile-web-app-capable\" content=\"yes\">\
%s\
<!-- <link rel=\"shortcut icon\" type=\"image/ico\" href=\"/images/w4.png\"> -->\
<!-- <script src=\"/js/globalik.js\"></script> -->\
<!-- [if lt IE9]<script src=\"http://html5shim.googlecode.com/svn/trunk/html5.js\"></script>\
<script src=\"http://css3-mediaqueries-js.googlecode.com/svn/trunk/css3-mediaqueries.js\"></script><![endif] -->\
", n.title !=NULL ? n.title : "main page", n.metatag !=NULL ? n.metatag : "");
char*s=kore_buf_stringify(b,NULL);
kore_free(b);
return s;	
}

struct kore_buf* rend(char*soup, char*me){
struct kore_buf*b;
b=kore_buf_alloc(128);
// compound literals
kore_buf_appendf(b,"<!DOCTYPE html>\n<html lang=\"en\">\
\n<head>%s</head>\n<body><!-- menu -->%s",
 html_head((struct obj_html_head){.title = "Globik", .metatag = str_meta_str()}),
 html_main_menu((struct obj_html_main_menu){.is_admin = is_admin, .buser = buser})
);
kore_buf_appendf(b,"<h1>hello %s</h1>", soup);
kore_buf_appendf(b," etwas etwas etwas.");
kore_buf_appendf(b,"<b>%s</b>", me);
kore_buf_appendf(b," something something...\n<footer>%s</footer></body></html>",
html_footer((struct obj_html_footer){.footer_str="Here must be a footer"}));
return b;	
}

int
page(struct http_request *req)
{
struct kore_buf*ldata=rend("soup","me");
http_response(req, 200, ldata->data, ldata->offset);
kore_free(ldata);
return (KORE_RESULT_OK);
}

char*str_meta_str(void){
char*s="<meta property=\"og:locale\" content=\"ru_Ru\" />\
<meta property=\"og:type\" content=\"website\" />\
<meta property=\"og:title\" content=\"Sex Videochat Alikon - тысячи моделей готовы пообщаться с тобой в любое время \
дня и ночи прямо из своих спален!\" />\
<meta property=\"og:description\" content=\"Эротический видеочат для взрослых.\" />\
<meta property=\"og:image\" content=\"http://example.com/bona.png\" />\
<meta property=\"og:url\" content=\"http://example.com\" />\
<meta property=\"og:site_name\" content=\"Alikon\" />\
<meta itemprop=\"name\" content=\"Sex Videochat Alikon\" />\
<meta itemprop=\"description\" content=\"Эротический видеочат для взрослых.\" />\
<meta itemprop=\"image\" content=\"http://example.com/bona.png\" />";
return s;	
}

char*html_footer(struct obj_html_footer n){
struct kore_buf*b;
b=kore_buf_alloc(128);
kore_buf_appendf(b,"%s",n.footer_str !=NULL ? n.footer_str : "simple deafault footer");
char*s=kore_buf_stringify(b,NULL);
kore_free(b);
return s;	
}

char*html_main_menu(struct obj_html_main_menu n){
struct kore_buf*b;
b=kore_buf_alloc(128);
kore_buf_appendf(b,"<h2>Main Menu</h2>");
if(n.buser){
kore_buf_appendf(b,"<a href=\"#\">this is for user: <span>%s</span></a><br>", n.buser);	
}
kore_buf_appendf(b,"%s", is_admin ? "<a href=\"#\">this is for admin.</a><br>" : "");
kore_buf_appendf(b,"%s", n.buser ? "<a href=\"#\">log out</a>" : "<a href=\"#\">log in</a>");
char*s=kore_buf_stringify(b,NULL);
kore_free(b);
return s;	
}
