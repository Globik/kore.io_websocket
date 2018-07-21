#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h> 
#include <postgresql/libpq-fe.h>

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
int done=1;
static void exit_nicely(PGconn*conn){
	printf(green "exit_nicely occured.\n" rst);
	printf("conn %p\n",conn);
	PQfinish(conn);
	printf("conn %p\n",conn);
	exit(1);
	}
static void han_sig(int n){
printf("n: %d\n",n);
printf(yellow "han_sig SIGINT occured.\n" rst);
done=0;	
}
int main(){
signal(SIGINT,han_sig);	
char*dbinfo;
const char*conninfo="dbname=postgres";
PGconn*conn;
PGresult*res;
PGnotify*notify;
conn=PQconnectStart(conninfo);
if(conn==NULL){printf(red "conn null\n" rst);exit(EXIT_FAILURE);}
//if(PQstatus(conn) !=CONNECTION_OK){printf(red "connection is NOT ok: %s\n" rst, PQerrorMessage(conn));exit_nicely(conn);}
switch(PQstatus(conn)){
	case CONNECTION_BAD:
	printf(red "connection bad: %s\n" rst,PQerrorMessage(conn));
	exit_nicely(conn);
	break;
	case CONNECTION_STARTED:
	printf(yellow "started\n" rst);
	break;
	case CONNECTION_MADE:
	printf(yellow "made\n" rst);
	break;
	case CONNECTION_AWAITING_RESPONSE:
	printf(yellow "awaiting response\n" rst);
	break;
	case CONNECTION_CHECK_WRITABLE:
	printf(yellow "check write\n" rst);
	break;
	case CONNECTION_CONSUME:
	printf(yellow "consume\n" rst);
	break;
	default:
	printf("connecting...\n");
	}
	
int u=PQsetnonblocking(conn,1);

printf("non blocking: %d\n",u);
int ud=PQisnonblocking(conn);
// 1 nonbl, 0 bl
printf("is non blocking? :%d\n",ud);
fd_set rfd,wfd;
int sock=PQsocket(conn);

if(sock<0){printf(red "failed to socket\n" rst);exit_nicely(conn);}
	while(done){
		usleep(200000);
	switch(PQconnectPoll(conn)){
				case PGRES_POLLING_ACTIVE:
				printf("pgres polling active\n");
				break;
				case PGRES_POLLING_READING:
				printf("pgres polling reading\n");
				break;
				case PGRES_POLLING_WRITING:
				printf("pgres polling writing\n");
				
				break;
				case PGRES_POLLING_OK:
				printf("pgres polling ok\n");
			
					done=0; 
				break;
				case PGRES_POLLING_FAILED:
				printf("pgres polling failed\n");
				if((dbinfo=PQerrorMessage(conn)) !=NULL){
				printf("err msg %s\n",dbinfo);
				//exit(EXIT_FAILURE);
				done=0;
				exit_nicely(conn);
				}
				default:
				printf("fuck knows\n");
				done=0;
				}
	}
	
	//if(done==0){
		int i=0;
		while(1){
			i++;
			usleep(200000);
			/*
FD_ZERO(&wfd);
FD_SET(sock,&wfd);
int ab=select(sock+1,NULL,&wfd,NULL,NULL);
printf("ab: %d\n",ab);
if(ab<0){
//if(errno==EINTR){printf(red "EINTR occurred.\n");continue;}
printf(red "select failed: %s\n" rst,strerror(errno));
exit_nicely(conn);	
}
*/
FD_ZERO(&rfd);
//FD_SET(sock,&rfd);
FD_SET(sock,&wfd);
int abu=select(sock+1,&rfd,&wfd,NULL,NULL);
printf("abu: %d\n",abu);
if(abu<0){
//if(errno==EINTR){printf(red "EINTR occurred.\n");continue;}
printf(red "select failed: %s\n" rst,strerror(errno));
exit_nicely(conn);	
}
printf("abu: %d\n",abu);
if(i==3){
	if(!PQsendQuery(conn,"listen banners")){
		fprintf(stderr,"failed to send query: %s\n",PQerrorMessage(conn));
		exit_nicely(conn);
		}
		int d=PQflush(conn);
		printf("flush %d\n",d);
		//FD_ZERO(&wfd);
		FD_SET(sock,&rfd);
		//break;
	}
if(i==10){exit_nicely(conn);return 0;}
}
		
	//	}
	
	/*
res=PQexec(conn,"LISTEN revents");
if(PQresultStatus(res) !=PGRES_COMMAND_OK){
	printf(red "listen command revents failed: %s\n" rst,PQerrorMessage(conn));
	PQclear(res);
	exit_nicely(conn);
	}
PQclear(res);


while(done){
int sock;
fd_set rfd;
sock=PQsocket(conn);
if(sock<0){printf(red "failed to socket\n" rst);exit_nicely(conn);}
FD_ZERO(&rfd);
FD_SET(sock,&rfd);
if(done==0){printf("000\n");}
int gu=select(sock+1,&rfd,NULL,NULL,NULL);
if(gu<0){
if(errno==EINTR){printf(red "EINTR occurred.\n");continue;}
printf(red "select failed: %s\n" rst,strerror(errno));
exit_nicely(conn);	
}
PQconsumeInput(conn);
while((notify=PQnotifies(conn)) !=NULL){
	printf(green "async came %s : %d : %s\n" rst, notify->relname,notify->be_pid,notify->extra);
	PQfreemem(notify);
	}
}
*/ 
	printf("Done.\n");
	PQfinish(conn);
	printf(yellow "**bye!***\n" rst);
	return 0;	
	
}
