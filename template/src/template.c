#include <kore/kore.h>
#include <kore/http.h>

struct obj_head{
char*metatag;
char*title;	
};

int		page(struct http_request *);
struct kore_buf*rend(char*,char*);
char*html_head(struct obj_head);

char*str_meta_str(void);

char*html_head(struct obj_head n){
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
",n.title !=NULL?n.title:"main page",n.metatag !=NULL?n.metatag:"");
char*s=kore_buf_stringify(b,NULL);
kore_free(b);
return s;	
}

struct kore_buf* rend(char*soup, char*me){
struct kore_buf*b;
//struct obj_head obj;
//obj.title="Globik";
//obj.metatag=NULL;//
//char* meta_str="\n<!-- <meta property=\"og:locale\" content=\"ru_Ru\" data-russian_test=\"Навальный\"/> -->";
b=kore_buf_alloc(128);
kore_buf_appendf(b,"<!DOCTYPE html>\n<html lang=\"en\">\
\n<head>%s</head>\n<body>",html_head((struct obj_head){.title="Globik",.metatag=str_meta_str()})
);
kore_buf_appendf(b,"<h1>hello %s</h1>", soup);
kore_buf_appendf(b," etwas etwas\
 etwas");

kore_buf_appendf(b,"<b>%s</b>", me);
kore_buf_appendf(b," something something...\n<footer>footer</footer></body></html>");
//char *s=kore_buf_stringify(b,NULL);
//printf("%s\n",s);

//kore_free(b); 
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
