// https://gist.github.com/revmischa/5384678
#include <stdio.h>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
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
	}else{printf("Connection started?\n");}
	if(status==CONNECTION_STARTED){printf("connection started!\n");}
mainloop(conn);
PQfinish(conn);
	}
	void exitclean(PGconn*conn){
		PQfinish(conn);
		exit(1);
		}
		void mainloop(PGconn*conn){
			fd_set rfds,wfds;
			struct timeval tv;
			int retval;
			int sock;
			int done=0;
			int connected=0;
			//int sentlisten=0;
			PostgresPollingStatusType connstatus;
			while(!done){
				printf("*WHILE_LOOP*\n");
				sock=PQsocket(conn);
				if(sock<0){
					printf("postgres socket is gone\n");
					exitclean(conn);
					}
					FD_ZERO(&rfds);
					FD_ZERO(&wfds);
					tv.tv_sec=2;
					tv.tv_usec=0;
					if(!connected){
						connstatus=PQconnectPoll(conn);
						switch(connstatus){
							case PGRES_POLLING_FAILED:
							fprintf(stderr,"pgconn failed %s\n",PQerrorMessage(conn));
							return;
							case PGRES_POLLING_WRITING:
							printf("PGRES_POLLING_WRITING\n");
							FD_SET(sock,&wfds);
							break;
							case PGRES_POLLING_READING:
							printf("PGRES_POLLING_READING\n");
							FD_SET(sock,&rfds);
							break;
							case PGRES_POLLING_OK:
							printf("PGRES_POLLING_OK\n");
							connected=1;
							initlisten(conn);
							break;
							}
						
						}else{
							if(connstatus==PGRES_POLLING_FAILED){printf("failed\n");}else if(connstatus==PGRES_POLLING_OK){
								printf("ok\n");}else if(connstatus==PGRES_POLLING_WRITING){
									printf("write\n");}else if(connstatus==PGRES_POLLING_READING){
										printf("read\n");}else{printf("unknown\n");}
							}
						if(connected){
						printf("CONNECTED=true\n");FD_ZERO(&wfds);FD_SET(sock,&rfds);
						}
						retval=select(sock+1,&rfds,&wfds,NULL,&tv);
						switch(retval){
							case -1:
							perror("select failed\n");
							done=1;
							break;
							case 0:
							printf("CASE 0\n");
							//break;
							
							default:
							if(!connected){
							printf("Not connected.\n");
							break;
						}
							if(FD_ISSET(sock,&rfds)){printf("DO HANDLE PG READ\n");handlepgread(conn);}
							//break;
							}
							printf("end of while\n");
				}
			}
void initlisten(PGconn*conn){
printf("Entering initlisten()\n");
char*quotedchannel=PQescapeIdentifier(conn,listenchannel,strlen(listenchannel));
char*query;
asprintf(&query,"LISTEN %s",quotedchannel);
int qs=PQsendQuery(conn,query);
PQfreemem(quotedchannel);
free(query);
if(!qs){
fprintf(stderr,"failed to send query: %s\n",PQerrorMessage(conn));
return;
}
}
void handlepgread(PGconn*conn){
	PGnotify*notify;
	PGresult*res;
	PQprintOpt opt;
	if(!PQconsumeInput(conn)){
	fprintf(stderr,"failed to consume input: %s\n",PQerrorMessage(conn));
	return;
	}
		
	while(res=PQgetResult(conn)){
		if(PQresultStatus(res) !=PGRES_COMMAND_OK){
			fprintf(stderr,"result err: %s\n",PQerrorMessage(conn));
			PQclear(res);
			return;
			}
			memset(&opt,'\0',sizeof(opt));
			PQprint(stdout,res,&opt);
			printf("got result\n");
		}
		while(notify=PQnotifies(conn)){
			fprintf(stderr,"notify of %s received from backend pid %d ,extra: %s\n",notify->relname,notify->be_pid,notify->extra);
			PQfreemem(notify);
			}
	}
