#include <string.h>
#include <re.h>
#include <pthread.h>

#define DEBUG_MODULE "remain"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

struct data{
pthread_t tid;
bool thread_started;
bool thread_exited;
unsigned tmr_called;
int err;	
};
static void tmr_handler(void*arg){
struct data*data=arg;
int err=0;
//if(0 !=pthread_equal(data->tid ,pthread_self())){DEBUG_WARNING("not equal\n");}
if(pthread_equal(data->tid,pthread_self())){
printf("thread IS EQUAL\n");//1 yes
}else{printf("thread NOT equal\n");}
++data->tmr_called;
printf("in timer body\n");
//out:
if(err)data->err=err;
//printf("before re_cancel()\n");
re_cancel();
printf("after re_cancel()\n");	
}
static void*thread_handler(void*arg){
printf("entering thread\n");
struct data*data=arg;
struct tmr tmr;
int err;
data->thread_started=true;
tmr_init(&tmr);

//libre_init();
//if(err){printf("libre_init failed\n");}
err=re_thread_init();
if(err){
DEBUG_WARNING("thread init: %m\n",err);
data->err=err;
return NULL;	
}
int su=pthread_equal(data->tid,pthread_self());
printf("su2: %d=\n",su);
tmr_start(&tmr,3,tmr_handler,data);
err=re_main(NULL);
if(err){printf("re_main err occured\n");data->err=err;}
tmr_cancel(&tmr);
tmr_debug();
re_thread_close();
data->thread_exited=true;
//libre_close();
	
printf("exite thread right now\n");
return NULL;	
}
static int test_remain_thread(void){
struct data data;
int i,err;
memset(&data,0,sizeof(data));
err=pthread_create(&data.tid,NULL,thread_handler,&data);
if(err){return err;}
for(i=0;i<500;i++){
if(data.tmr_called){printf("data.tmr_called yes, i: %d\n",i);break;}
if(data.err)break;
sys_msleep(1);	
}

int a=pthread_join(data.tid,NULL);
printf("a : %d\n",a);
if(data.err)return data.err;
printf("%d\n",data.thread_started);
printf("%d\n",data.thread_exited);
if(1 == data.tmr_called)DEBUG_WARNING("one time tmr called: %d\n",data.tmr_called);
DEBUG_WARNING("data.err: %d\n",data.err);
out:
return err;	
}
int main(){
int err=0;
err=test_remain_thread();
if(err){printf("err: %d\n",err);}
printf("***bye!***\n");
return 0;	
}
