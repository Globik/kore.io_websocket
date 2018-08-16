#include <stdlib.h>
#include <stdio.h>
#include <string.h>
struct n_header_t{
char*title;
char*user;	
};

const char*fuckery="<html><head><title>%s</title></head><body><h1>%s</h1>\
</body></html>";
const char*fuckery2="<html><head><title>%s</title></head><body><h1>%s</h1></body></html>";

char*headi(char*s){
char *result=NULL;
if(asprintf(&result,"<head><title>%s</title></head>", (s&&s !=NULL) ? s : "title") < 0)return NULL;
return result;	
}

const char*buser(const char*n){return n;}
const char*mm(struct n_header_t n){
const char*result=NULL;
const char*entry="<html>%s<body><h1>%s</h1></body></html>";
if(asprintf(&result,entry, n.title, buser(n.user ? n.user : "a fucker")) < 0)return NULL;
return result;	
}
int main(){
	char*hea=headi("coder");
struct n_header_t f={hea,NULL};
const char *su=mm(f);
//char *su=mm("coder","linux");
printf("su: %s\n",su !=NULL ? su : "the fuck you do here");
free(hea);
free((void*)su);
f.title=headi("sifka");
f.user="feoder";

su=mm(f);
printf("su2: %s\n",su !=NULL ? su : "the fuck you do here2");
free(f.title);
free((void*)su);
int d=strlen(fuckery);
int d2=strlen(fuckery2);
printf("len of entry: %d\n",d);
printf("len of entry2: %d\n",d2);
return 0;	
}
