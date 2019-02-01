#include <kore/kore.h>
#include <kore/http.h>
#include <kore/tasks.h>
#include <re.h>

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"
// dsj
int		page(struct http_request *);
int init(int);
int libre_loop(struct kore_task*);
void data_av(struct kore_task*);
void sig_ha(int);
struct kore_task task;

int init(int state){
if(state==KORE_MODULE_UNLOAD)return (KORE_RESULT_ERROR);
kore_task_create(&task, libre_loop);
kore_task_bind_callback(&task,data_av);
kore_task_run(&task,1);
return (KORE_RESULT_OK);	
}   

int page(struct http_request *req)
{
	http_response(req, 200, NULL, 1);
	printf(green "page!\n" rst);
	//re_thread_enter();
	//re_thread_leave();
	//re_cancel();
	//re_thread_enter();
	//libre_close();
	return (KORE_RESULT_OK);  
}

void sig_ha(int sig){
	printf("terminating %d\n",sig);
	
re_cancel();	 
}
int libre_loop(struct kore_task*t){
kore_task_channel_write(t,"mama\0",5);
int err; 
err=libre_init();
if(err){
printf("no l in\n");
//goto out;	
}
err=re_main(NULL);
if(err){
printf("err %d\n",err);	  
}
//re_cancel();
libre_close();
printf("kuku\n");
kore_task_channel_write(t,"papa\0",5);
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

}
