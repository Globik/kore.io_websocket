#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#define socket_name "/home/globik/fuck"
#define buffer_size 512
int main(int argc,char*argv[]){
struct sockaddr_un addr;

	int i;
	int ret;
	int data_socket;
	char buffer[buffer_size];
	data_socket=socket(AF_UNIX,SOCK_SEQPACKET,0);
	if(data_socket==-1){
	perror("socket");
		exit(EXIT_FAILURE);
	}
	memset(&addr,0,sizeof(struct sockaddr_un));
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path,socket_name,sizeof(addr.sun_path)-1);
	ret=connect(data_socket,(const struct sockaddr*)&addr,sizeof(struct sockaddr_un));
if(ret==-1){
fprintf(stderr,"the server is down\n");
	exit(EXIT_FAILURE);
}
	for(i=1;i<argc;++i){
	ret=write(data_socket,argv[i],strlen(argv[i])+1);
		if(ret==-1){
		perror("write");
break;
		}
	}
	strcpy(buffer,"end");
	ret=write(data_socket,buffer,strlen(buffer)+1);
	if(ret==-1){
	perror("write2");
		exit(EXIT_FAILURE);
	}
	ret=read(data_socket,buffer,buffer_size);
	if(ret==-1){
	perror("read");
		exit(EXIT_FAILURE);
	}
	buffer[buffer_size-1]=0;
	printf("result=%s\n",buffer);
	close(data_socket);
	exit(EXIT_SUCCESS);
}