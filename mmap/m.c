#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
//#include <synch.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
typedef struct{
	sema_t msema;
	int num;
} buf_t;
int main(){
int i,j,fd;
	buf_t *buf;
	fd=open("/dev/zero",O_RDWR);
	buf=(buf_t*)mmap(NULL,sizeof(buf_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	sema_init(&buf->msema,0,USYNC_PROCESS,0);
	if(fork()==0){
	for(j=0;j<5;j++){
	printf("child pid %d: waiting...\n",getpid());
		sema_wait(&buf->msema);
		printf("child pid %d: decrementing...\n",getpid());
		
	}
		printf("child pid %d: exit...\n",getpid());
		exit(0);
	}
	sleep(2);
	for(i=0;i<5;i++){
		printf("parent pid %d: posting...\n",getpid());
	sema_post(&buf->msema);
		printf("parent pid %d: exit...\n",getpid());
		sleep(1);
	}
	if(munmap(buf_t,sizeof(buf_t)<-1){printf("%s\n",errno_s);}
	return(0);
}

// gcc m m.c