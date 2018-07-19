// https://www.postgresql.org/docs/9.1/static/libpq-example.html
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
const char*conninfo="dbname=postgres";
PGconn*conn;
PGresult*res;
PGnotify*notify;
conn=PQconnectdb(conninfo);
if(PQstatus(conn) !=CONNECTION_OK){
printf(red "connection is NOT ok: %s\n" rst, PQerrorMessage(conn));
exit_nicely(conn);	
}
res=PQexec(conn,"LISTEN revents");
if(PQresultStatus(res) !=PGRES_COMMAND_OK){
	printf(red "listen command revents failed: %s\n" rst,PQerrorMessage(conn));
	PQclear(res);
	exit_nicely(conn);
	}
PQclear(res);

res=PQexec(conn,"LISTEN on_coders");
if(PQresultStatus(res) !=PGRES_COMMAND_OK){
	printf(red "listen command on_coders failed: %s\n" rst,PQerrorMessage(conn));
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
	printf("Done.\n");
	PQfinish(conn);
	printf(yellow "**bye!***\n" rst);
	return 0;	
	
}
