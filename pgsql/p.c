#include <libpq-fe.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
//#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "lwan-coro.h"
// https://habr.com/post/350140/
#define sock_poll_timeout 100
typedef enum{
	disconnected=0,
	conn_polling,
	conn_reading,
	conn_writing,
	ready,
	query_sent,
	query_flushing,
	query_busy,
	query_reading,
	closing,
	error}pq_state;
	
	typedef enum{
		no_error=0,
		allocation_fail,
		polling_fail,
		reading_fail,
		writing_fail,
		timeout_fail
	} pq_error;
	struct pqconn_s {
		pq_state state;
		PGconn*conn;
		unsigned long start;
		long timeout;
		pq_error error;
		};
		unsigned long time_ms(void){
			struct timespec tp;
			clock_gettime(CLOCK_MONOTONIC,&tp);
			return (tp.tv_sec*1000+tp.tv_nsec/1000000);
			}
			
struct fact{
struct pqconn_s s;
const char*conninfo;
long timeout;
};

//struct pqconn_s *s;
int try_socket(int socket_fd,int rw,struct coro*coro){
				fd_set fset;
				struct timeval sock_timeout;
				sock_timeout.tv_sec=0;
				sock_timeout.tv_usec=sock_poll_timeout;
				FD_ZERO(&fset);
				FD_SET(socket_fd,&fset);
				printf("try sock: %d\n",rw);
				setsockopt(socket_fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&sock_timeout,sizeof(struct timeval));
				//return 
				select(socket_fd+1,((!rw)?&fset:NULL),((rw) ? &fset:NULL),NULL,&sock_timeout);
				coro_yield(coro,0);
}
				
int pgsql_connection_start(struct coro*coro,void*data)//, const char*conninfo,struct pqconn_s*s,long timeout){
{
//struct fact*f=(struct fact*)data;
const char*conninfo="dbname=postgres";//f->conninfo;
struct pqconn_s *s=(struct pqconn_s*)data;
long timeout=15000;
printf("what?\n");
/*
if(s==NULL){
printf("!s\n");
coro_yield(coro,0);	
__builtin_unreachable();
return 0;}
*/ 
/*
if(!conninfo){
printf("!conninfo\n");
s.error=allocation_fail;
return 0;
}
*/
s->conn=PQconnectStart(conninfo);

s->state=conn_polling;
s->start=time_ms();
s->timeout=timeout;
s->error=no_error;
ConnStatusType status;
status=PQstatus(s->conn);
if(status==CONNECTION_BAD){
printf("bad?\n");
s->state=error;
s->error=polling_fail;
coro_yield(coro,0);
__builtin_unreachable();
return 0;
}

coro_yield(coro,1);
return 1;
					
}
int pgsql_send_query(struct pqconn_s*s,const char*command,long timeout){
if(s->state !=ready){
return 0;
}
if(!PQsendQuery(s->conn,command)){
return 0;
	}
PQsetnonblocking(s->conn,0);
s->state=query_flushing;
s->start=time_ms();
s->timeout=timeout;
s->error=no_error;
return 1;
}

int pgsql_ev_loop(struct coro*coro,void*data)//(struct pqconn_s*s)
{
struct pqconn_s*s=(struct pqconn_s*)data;
if((s->state==disconnected)||(s->state==ready))return;
//while(s->state !=ready)
if((time_ms()-s->start)>s->timeout){
s->state=closing;
s->error=timeout_fail;
}
if(s->state==conn_polling){
printf("conn_polling\n");
PostgresPollingStatusType poll_result=PQconnectPoll(s->conn);
if(poll_result==PGRES_POLLING_WRITING){printf("PGRES_POLLING_WRITING\n");s->state=conn_writing;}
if(poll_result==PGRES_POLLING_READING){
printf("PGRES_POLLING_READING\n");
s->state=conn_reading;
}
if(poll_result==PGRES_POLLING_FAILED){
printf("polling failed\n");
s->state=error;
s->error=polling_fail;
}
if(poll_result==PGRES_POLLING_OK){
printf("PGRES_POLLING_OK\n");s->state=ready;
coro_yield(coro,1);

return 1;
}	
}
if(s->state==conn_reading){
printf("what the f\n");
int sock_state=try_socket(PQsocket(s->conn),0,coro);
if(sock_state==-1){
printf("-1 try sock\n");
s->error=reading_fail;
s->state=closing;	
coro_yield(coro,0);
__builtin_unreachable();
return 0;
}
if(sock_state>0){
printf("conn_polling in reading?\n");
s->state=conn_polling;
//coro_yield(coro,1);
//return 1;
}	
}
if(s->state==conn_writing){
int sock_state=try_socket(PQsocket(s->conn),1,coro);
if(sock_state==-1){
s->error=writing_fail;
s->state=closing;	
}
if(sock_state>0){printf("conn_polling in writing?\n");s->state=conn_polling;}	
}
if(s->state==closing){
PQfinish(s->conn);
s->state=error;	
}
if(s->state== query_flushing){
int flush_res=PQflush(s->conn);
if(0==flush_res)s->state=query_reading;
if(-1==flush_res){
s->error=writing_fail;
s->state=closing;	
}	
}
if(s->state==query_reading){
	printf("query reading?\n");
int sock_state=try_socket(PQsocket(s->conn),0,coro);
if(sock_state==-1){
s->error=reading_fail;
s->state=closing;	
}
if(sock_state>0)s->state=query_busy;	
}
if(s->state==query_busy){
if(!PQconsumeInput(s->conn)){
s->error=reading_fail;
s->state=closing;	
}
if(PQisBusy(s->conn)){
printf("pqisbusy?\n");	
s->state=query_reading;	
}else{
s->state=ready;
}	
}	
//coro_yield(coro,1);
//__builtin_unreachable();
return 0;
}
				
int main(){
//struct fact f;
struct pqconn_s s;
struct coro_switcher switcher;
//f.conninfo="dbname=postgres";
//f.s=si;
//f.timeout=15000;
struct coro*coro = coro_new(&switcher, pgsql_connection_start, &s);
struct coro*coro2= coro_new(&switcher, pgsql_ev_loop, &s);
//pgsql_connection_start("dbname=postgres",&s,15000);
int a=coro_resume(coro);
if(a==0){
	//exit(1);
	}
printf("a %d\n",a);
//while((s.state !=error)&&(s.state !=ready)){
printf("ev loop 1?\n");
//pgsql_ev_loop(&s);
int b;
coro_resume(coro2);
while(b=coro_resume(coro2)){
printf("b %d\n",b);
}
coro_resume(coro2);
if(s.state==error){
perror("db connection failed\n");
return 1;
}
printf("connection %d\n",s.state);
/*
			pgsql_send_query(&s,"SELECT*FROM coders",50000);
			while((s.state !=error)&&(s.state !=ready)){
				//printf("ev loop 2?\n");
			pgsql_ev_loop(&s);
				}
				if(s.state==error){
					perror("query failed.\n");
					return 1;
					}
PGresult *res;
int rec_count;
int row;
int col;
res=PQgetResult(s.conn);
if(PQresultStatus(res) !=PGRES_TUPLES_OK){
	perror("we did not get any data\n");
	return 1;
	}
	rec_count=PQntuples(res);
	printf("received %d records\n",rec_count);
	for(row=0;row<rec_count;row++){
		//for(col=0;col<3;col++){
			printf("%s\n",PQgetvalue(res,row,0));
			//}
			//puts("");
		}
		PQclear(res);
		* */
		PQfinish(s.conn);
		coro_free(coro);
		coro_free(coro2);
		coro=NULL;coro2=NULL;
return 0;
}
