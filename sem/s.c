#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <string.h>
// gcc -o s s.c -lpthread
struct shared{
	sem_t mutex;
	int count;
};
int main(){
int i;
	struct shared*ptr;
	ptr=(struct shared*)mmap(NULL,sizeof(struct shared),PROT_READ | PROT_WRITE,MAP_ANONYMOUS | MAP_SHARED,0,0);
	if(sem_init(&ptr->mutex,1,6) !=0){printf("init err\n");exit(1);}
	//setbuf(stdout,NULL);
	ptr->count=0;
	if(fork()==0){
	for(i=0;i<3;i++){
	sem_wait(&ptr->mutex);
		//ptr->count++;
		printf("child %d\n",++ptr->count);
		sem_post(&ptr->mutex);
	}
		exit(0);
	}
	//else{
	for(i=0;i<3;i++){
	sem_wait(&ptr->mutex);
	printf("parent %d\n",++ptr->count);
	sem_post(&ptr->mutex);
	}
	//}
	sem_destroy(&ptr->mutex);
	if(munmap(ptr,sizeof(struct shared))<-1){printf("some err in munmap\n");}
	exit(0);
}