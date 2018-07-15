// www.fireproject.jp/feature/postgresql/programming_libpq/connect_nowait.c.html
#include <stdio.h>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>
#include <sys/select.h>

int main(){
	PGconn*connection;
	int status;
	char*dbinfo;
	int flag;
	fd_set fdset;
	int fd;
	connection=PQconnectStart("dbname=postgres");
	if(connection==NULL){printf("pqconnect start failed\n");exit(EXIT_FAILURE);}
	status=PQstatus(connection);
	switch(status)
	{
		case CONNECTION_OK:
		printf("connection ok\n");
		flag=0;
		break;
		case CONNECTION_BAD:
		printf("connection bad\n");
		if((dbinfo=PQerrorMessage(connection)) !=NULL){printf("err msg %s\n",dbinfo);}
		exit(EXIT_FAILURE);
		case CONNECTION_STARTED:
		printf("connection started\n");
		flag=1;
		break;
		case CONNECTION_MADE:
		printf("connection made\n");
		flag=1;
		break;
		case CONNECTION_AWAITING_RESPONSE:
		printf("connection awaiting response\n");
		flag=1;
		break;
		case CONNECTION_AUTH_OK:
		printf("connection auth ok\n");
		flag=1;
		break;
		case CONNECTION_SETENV:
		printf("connection set env\n");
		flag=1;
		break;
		default:
		printf("unexpect\n");
		exit(EXIT_FAILURE);
		}
		while(flag){
			printf("flag: %d\n",flag);
			switch(PQconnectPoll(connection)){
				case PGRES_POLLING_ACTIVE:
				printf("pgres polling active\n");
				break;
				case PGRES_POLLING_READING:
				printf("pgres polling reading\n");
				FD_ZERO(&fdset);
				fd=PQsocket(connection);
				FD_SET(fd,&fdset);
				int a=select(fd+1,&fdset,NULL,NULL,NULL);
				printf("a: %d\n",a);
				if(FD_ISSET(fd,&fdset)){printf("fd %d\n",fd);}
				break;
				case PGRES_POLLING_WRITING:
				printf("pgres polling writing\n");
				FD_ZERO(&fdset);
				fd=PQsocket(connection);
				FD_SET(fd,&fdset);
				select(fd+1,NULL,&fdset,NULL,NULL);
				if(FD_ISSET(fd,&fdset)){printf("fd %d\n",fd);}
				break;
				case PGRES_POLLING_OK:
				printf("pgres polling ok\n");
				flag=0;
				break;
				case PGRES_POLLING_FAILED:
				printf("pgres polling failed\n");
				if((dbinfo=PQerrorMessage(connection)) !=NULL){printf("err msg %s\n",dbinfo);exit(EXIT_FAILURE);}
				
				}
				
			}
	PQfinish(connection);
	exit(EXIT_SUCCESS);
	}
