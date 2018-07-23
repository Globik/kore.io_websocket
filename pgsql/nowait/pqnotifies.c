// https://gist.github.com/revmischa/5384678
#include <stdio.h>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
int fuck=0;
int done=0;

const char*listenchannel="revents";
void mainloop(PGconn*conn);
void exitclean(PGconn*conn);
void handlepgread(PGconn*conn);
void initlisten(PGconn*conn);
int main(){
const char*conninfokeys[]={"dbname",NULL};
const char*conninfovalues[]={"postgres",NULL};
PGconn*conn=PQconnectStartParams(conninfokeys,conninfovalues,0);
ConnStatusType status=PQstatus(conn);
if(status==CONNECTION_BAD){
fprintf(stderr, red "connection database failed %s\n" rst, PQerrorMessage(conn));
exitclean(conn);
}else if(status==CONNECTION_STARTED){
printf("connection started!\n");
}else if(status==CONNECTION_MADE){
printf(green "Connection made.\n" rst);	
}else{printf(yellow "connecting...\n");}
mainloop(conn);
PQfinish(conn);
printf(green "***bye!***\n" rst);
}

void exitclean(PGconn*conn){
printf(yellow "exitclean(conn) occured.\n");
PQfinish(conn);
done=1;
exit(1);
}
static void han_sig(int n){
printf("n: %d\n",n);
printf(yellow "han_sig SIGINT occured.\n" rst);
done=1;	
}
void foo(int n){
	printf(red "ON TERMINATION\n" rst);
	}
void mainloop(PGconn*conn){
fd_set rfds,wfds;
int retval;
int sock;
signal(SIGINT,han_sig);	
signal(SIGHUP,foo);
int connected=0;
int connected2=0;
	
int u=PQsetnonblocking(conn,1);
printf("non blocking: %d\n",u);
int ud=PQisnonblocking(conn);
// 1 nonbl, 0 bl
printf("is non blocking? :%d\n",ud);

sock=PQsocket(conn);	
if(sock<0){
printf("postgres socket is gone\n");
exitclean(conn);
}
					
PostgresPollingStatusType connstatus;
while(!done){
printf("*WHILE_LOOP*\n");
//usleep(20000);
//FD_ZERO(&rfds);
//FD_ZERO(&wfds);
if(!connected){
connstatus=PQconnectPoll(conn);
switch(connstatus){
case PGRES_POLLING_FAILED:
fprintf(stderr,"pgconn failed %s\n",PQerrorMessage(conn));
done=1;
exitclean(conn);
break;
case PGRES_POLLING_WRITING:
printf("PGRES_POLLING_WRITING\n");
//	FD_SET(sock,&wfds);
break;
case PGRES_POLLING_READING:
printf("PGRES_POLLING_READING\n");
FD_SET(sock,&rfds);
break;
case PGRES_POLLING_OK:
printf(green "PGRES_POLLING_OK\n" rst);
connected=1;
initlisten(conn);
break;
}
}else{
	if(connstatus==PGRES_POLLING_OK){printf(red "poll OK\n" rst);
		printf(green " done: %d\n" rst,done);
		}else if(connstatus==PGRES_POLLING_FAILED){
		printf(red "pgres_polling_failed\n" rst);
		}else{printf(green "DEFALTI\n" rst);}
	}




//end resetting


if(connected){
printf("CONNECTED=true\n");

if(fuck==1){
//FD_ZERO(&rfds);
//FD_SET(sock,&rfds);
printf(green "FUCK?: %d\n" rst,fuck);
}
/*if(FD_ISSET(sock,&rfds)){
printf(yellow "DO HANDLE PG READ: retval: %d socket: %d\n" rst,retval,sock);
//if(retval==-1){done=1;exitclean(conn);}
handlepgread(conn);
}
*/ 
}
printf("BEFORE SELECT\n");
retval=select(sock+1,&rfds,NULL,NULL,NULL);
printf("after select\n");
switch(retval){
case -1:
//perror("select failed\n");
if(errno==EINTR){
printf(red "EINTR occurred.\n");
}
printf(red "done??\n" rst);

done=1;
break;
case 0:
printf("CASE 0\n");
break;
default:
if(!connected){
printf("Not connected.\n");
break;
}
if(FD_ISSET(sock,&rfds)){printf(yellow "DO HANDLE PG READ: retval: %d %d\n" rst,retval,sock);handlepgread(conn);}
printf("default\n");
}
//break;
printf("end of while\n");
}
}

void initlisten(PGconn*conn){
printf("Entering initlisten()\n");
char*quotedchannel=PQescapeIdentifier(conn,listenchannel,strlen(listenchannel));
char*query;
asprintf(&query,"LISTEN %s",quotedchannel);
int qs=PQsendQuery(conn,"LISTEN revents;LISTEN on_coders"/*query*/);
PQfreemem(quotedchannel);
free(query);
if(!qs){
fprintf(stderr,"failed to send query: %s\n",PQerrorMessage(conn));
return;
}
}
void handlepgread(PGconn*conn){
printf("entering handlepgread(conn)\n");
PGnotify*notify;
PGresult*res;
//PQprintOpt opt={0};
if(!PQconsumeInput(conn)){
	printf("hhhh\n");
fprintf(stderr,"failed to consume input: %s\n",PQerrorMessage(conn));
//done=1;
//exitclean(conn);
return;
}
printf("before result\n");
/*
if(NULL==PQgetResult(conn)){
printf("result NULL: %s\n",PQerrorMessage(conn));
ConnStatusType status=PQstatus(conn);
if(status==CONNECTION_BAD){printf("BAAAAAAAD\n");}else{printf("status unknown\n");}
	
	}else{printf("result is not null.\n");}
	*/
	/*
switch(PQresultStatus(PQgetResult(conn))){
	case PGRES_FATAL_ERROR:
	printf("fatali erri: %s\n",PQresultErrorMessage(PQgetResult(conn)));
	break;
	default:
	printf("fuck know aaa\n");
	}
	*/ 

while((res=PQgetResult(conn)) !=NULL){
if(PQresultStatus(res) !=PGRES_COMMAND_OK){
fprintf(stderr,"result err: %s\n",PQerrorMessage(conn));
PQclear(res);
return;
}
//memset(&opt,'\0',sizeof(opt));
//PQprint(stdout,res, &opt);
printf("got result: %s\n",PQresStatus(PQresultStatus(res)));
printf("returns rows: %d\n",PQntuples(res));
printf("cols: %d\n",PQnfields(res));
printf("cmd status: %s\n",PQcmdStatus(res));
fuck=1;
PQclear(res);
}

printf("before notify\n");
while(notify=PQnotifies(conn)){
fprintf(stderr,yellow "notify of %s received from backend pid %d ,extra: %s\n" rst, notify->relname, notify->be_pid, notify->extra);
PQfreemem(notify);
}
printf("END\n");
}
