#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
#include <re.h>

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
// d
int		page(struct http_request *);
int init(int);
int libre_loop(struct kore_task*);
void data_av(struct kore_task*);

void tmr_handler(void*);


void mqueue_handler(int,void*,void*);
void parse_remote_description(int,void*);
int get_json_stdin(struct odict** const dictp);

struct kore_task task;  

struct mqueue*mq=NULL;
int chao=0;
void kore_worker_teardown(void){
kore_log(LOG_INFO,yellow "teardown" rst);

if(chao==0){
int s=mqueue_push(mq,43,NULL);
printf(green "s %d\n" rst,s);
usleep(5000);
//usleep(500000);
}
}

void mqueue_handler(int f,void*data,void*arg){
printf(green "mqueue_handler occured\n" rst);
re_cancel();	
}

int init(int state){
if(state==KORE_MODULE_UNLOAD)return (KORE_RESULT_ERROR);

kore_task_create(&task, libre_loop);
kore_task_bind_callback(&task,data_av);
kore_task_run(&task,0);

return (KORE_RESULT_OK);	
}   

int page(struct http_request *req)
{
http_response(req, 200, NULL, 0);
printf(green "page!\n" rst);

if(mq!=NULL){
printf(green "mq is not null\n" rst);
int s=mqueue_push(mq,43,NULL);
printf(green "send: %d\n" rst,s);//0 if ok
}
return (KORE_RESULT_OK);  
}


int libre_loop(struct kore_task*t){
kore_task_channel_write(t,"mama\0",5);

struct tmr tmr;
tmr_init(&tmr);
int err; 
err=libre_init();
if(err){
printf("libre init failed\n");
//goto out;	
return (KORE_RESULT_OK);
}
err=mqueue_alloc(&mq,mqueue_handler,NULL);
if(err){
printf(red "mqueue_alloc failed\n" rst);
libre_close();
return (KORE_RESULT_OK);
}

tmr_start(&tmr,3000,tmr_handler,t);
fd_listen(STDIN_FILENO, FD_READ, parse_remote_description,NULL);
err=re_main(NULL);
if(err){
printf("err %d\n",err);	 
mem_deref(mq);
tmr_cancel(&tmr); 
libre_close();
return (KORE_RESULT_OK);
}

fd_close(STDIN_FILENO);
mem_deref(mq);
libre_close();
tmr_cancel(&tmr);
tmr_debug();

chao=1;
printf(green "*** bye from a thread! ***\n" rst);
return (KORE_RESULT_OK);
}
void data_av(struct kore_task *t){
size_t len;
u_int8_t buf[BUFSIZ];
if(kore_task_finished(t)){
kore_log(LOG_NOTICE,"Task finished.");
return;

}
len=kore_task_channel_read(t,buf,sizeof(buf));
if(len > sizeof(buf))printf("len great than buf\n");
kore_log(LOG_NOTICE,"Task msg: %s",buf);
printf(yellow "LEN: %d\n" rst,len);
}
void tmr_handler(void*arg){
struct kore_task*t2=arg;
printf("timer handler occured!\n");
kore_task_channel_write(t2,"suka\0",5);
printf("before re_cancel()\n");
re_cancel();
printf("after re_cancel()\n");	
}
void parse_remote_description(int fl,void*arg){
struct odict*dict=NULL;
char*type_str;
int err;
err=get_json_stdin(&dict);
if(err)printf(red "get json stdin failed\n" rst);
mem_deref(dict);
}
int get_json_stdin(struct odict** const dictp){
// rawrtc tools https://github.com/rawrtc/rawrtc DataChannel API ORTC and WebRTC
// peer-connection 1 host => offer, copy sdp and put it into another terminal, where command peer-connection 0 host
// internet access must be active
char buffer[255];
size_t length;
if(!fgets((char*)buffer,255,stdin)){
printf(red "fgets failed\n" rst);
return 1;
}
	length=strlen(buffer);
if(length==1 && buffer[0]=='\n'){
printf(red "no value\n" rst);
return 1;
}
int a=json_decode_odict(dictp,16,buffer,length,3);
printf("a %d\n",a);
printf("buffer: %s\n",buffer);
// "{\"a\":\"suuka\"}"
return 0;
}
