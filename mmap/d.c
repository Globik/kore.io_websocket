#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <fcntl.h>

#include <string.h>
#include <unistd.h>
void* cs(size_t s){
int p=PROT_READ | PROT_WRITE;
	int v=MAP_ANONYMOUS | MAP_SHARED;
	return mmap(NULL,s,p,v,0,0);
}
int main(){
char * m="hello";
	char *p="goodbye";
void* x=cs(128);
	memcpy(x,m,sizeof(m));
	int pid=fork();
	if(pid==0){
	printf("child read: %s\n",(char*)x);
		memcpy(x,p,sizeof(p));
		printf("child wrote: %s\n",(char*)x);
	}else{
	printf("parent read: %s\n",(char*)x);
		sleep(1);
		printf("after 1s parent read: %s\n",(char*)x);
	}
	if(munmap(x,128)<-1){printf("some err\n");}
	exit(0);
	
}
// gcc -o d d.c
