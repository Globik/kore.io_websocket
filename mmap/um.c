#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#define socket_name "/home/globik/fuck"
#define buffer_size 512
int current=0;
int sent=0;
int total=10;
int answered=0;
int main(){
struct sockaddr_un addr;

	int i;
	int ret;
	int data_socket;
	char buffer[512];
	data_socket=socket(AF_UNIX, SOCK_SEQPACKET/* | O_NONBLOCK*/, 0);
	if(data_socket==-1){
	perror("socket");
	exit(EXIT_FAILURE);
	}
	memset(&addr,0,sizeof(struct sockaddr_un));
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path,socket_name,sizeof(addr.sun_path)-1);
	ret=connect(data_socket,(const struct sockaddr*)&addr,sizeof(struct sockaddr_un));
	
	int flag=fcntl(data_socket,F_GETFL,0);
	if(flag<0){printf("some flag1 err\n");}
	flag |=O_NONBLOCK;
	if(fcntl(data_socket,F_SETFL,flag)<0){printf("some flag2 err\n");}
	if(ret==EINPROGRESS){printf("ret einprogress\n");}else{printf("kuku : %d\n",ret);}
if(ret==-1){
fprintf(stderr,"the server is down\n");
exit(EXIT_FAILURE);
}
	
	fd_set read_fds;
	fd_set write_fds;
	fd_set except_fds;
	printf("before while\n");
	while(1){
		printf("entering while\n");
		
	FD_ZERO(&read_fds);
	FD_SET(data_socket,&read_fds);
	FD_ZERO(&write_fds);
	//if(server->send_buffer.current>0)
		
	//if(current==0){
		// to send data
	//if(answer==1)
		FD_SET(data_socket,&write_fds);
//	}else if(current>0){
	//if(answered==1){FD_SET(data_socket,&write_fds);}
		answered=0;
	//}
		//else{
		// we can read
		//FD_SET(data_socket,&read_fds);
	//}
	FD_ZERO(&except_fds);
	FD_SET(data_socket, &except_fds);
	
		printf("before activity\n");
		int activity=select(data_socket+1, &read_fds,&write_fds,&except_fds, NULL);
		printf("ACTIVITY: %d\n", activity);
		switch(activity){
			case -1:
				printf("select -1\n");
				perror("select errooooooooooooooooorr");
				close(data_socket);
				exit(1);
			case 0:
				printf("select returns 0\n");
				close(data_socket);
				exit(1);
			default:
				if(FD_ISSET(data_socket,&read_fds)){
				printf("received from server somthing\n");
					/*
	int papa=read(data_socket,buffer,buffer_size);
	if(papa==-1){
	perror("read");
	exit(EXIT_FAILURE);
	}
	printf("PAPA: %d\n",papa);
	buffer[papa]='\0';
	printf("result=%s\n",buffer);
					total=papa;
					sent=0;
					//current=0;
					//break;
					//return 2;
	//close(data_socket);
	//exit(0);
					answered=1;
					//current=0;
					*/
	}
				
	if(FD_ISSET(data_socket,&write_fds)){
	printf("something to write\n");
		do{continue;}while(1);
		/*
	ssize_t mama=0;	
	size_t send_total=0;
	size_t len_to_send;
	//do{
	if(current==0)mama=send(data_socket,"suka\0", 5,0);
	if(mama==-1){
	printf("write err\n");
	perror("write");
		close(data_socket);
		exit(1);
	}
		printf("mama: %d\n",mama);
		//mama=0;
		
	
		if(mama==5){printf("mama: %d\n",mama);
					//FD_SET(data_socket,&read_fds);
					//continue;
					//break;
					//return 0;
					total=0;
					sent=0;
					
				   }else{sent+=mama;}
					printf("after write. MAMA: %d\n",mama);
		
					//close(data_socket);
					//exit(1);
		if(mama>0)current++;
		//current=0;
		printf("CURRENT: %d\n",current);
		printf("MAMA: %d\n",mama);
		//}while ( current<5);
		//return 1;
		if(mama==0){
			//return 1;
			//continue;
			//break;
			//FD_SET(data_socket,&write_fds);
			
		}
		*/
				}
				
				if(FD_ISSET(data_socket,&except_fds)){
				printf("something error report\n");
					close(data_socket);
					exit(1);
				}
				
		}
		printf("and we are still waiting for server activity.\n");
		//close(data_socket);
		//exit(0);
	}
	
	/*
	ret=write(data_socket,"suka", strlen("suka")+1);
	if(ret==-1){
	printf("write err\n");
	perror("write");
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
	*/
	return 0;
}