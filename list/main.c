#include "stdio.h"
#include "globi_ee.h"

struct fucker{
char* who;	
};
void on_hello(void*, void*);
int main(){
const char* ev_str = "hello";
const char* fake_ev = "fake";
struct fucker foo;
foo.who = "papa";
ee_t *ee=ee_new();
if(ee==NULL){
	printf("ee is NULL\n");
	return 0;
	}
ee_on(ee, ev_str, on_hello, (void*)&foo);
ee_on(ee, ev_str, on_hello, (void*)&foo);
ee_emit(ee, ev_str, "mama");
ee_emit(ee, ev_str, "sister");

ee_emit(ee, fake_ev, "brother");
ee_remove_listener(ee, ev_str, on_hello);
ee_destroy(ee);
printf("*** Buy! ***\n");
return 0;	
}

void on_hello(void* d, void* b){
printf("on_hello() occured.\n");
printf("data d: %s\n",(char*)d);
 struct fucker* c=(struct fucker*)b;
printf("data b: %s\n", c->who);	
}
