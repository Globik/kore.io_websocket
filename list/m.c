#include <stdio.h>
#include <ee.h>

void on_hello(void*d){
printf("on_hello: %s\n",(char*)d);	
	}
int main(){
const char* ev_str="hello";
	
ee_t *ee=ee_new();
ee_once(ee, ev_str, on_hello);
ee_on(ee, ev_str, on_hello);
ee_emit(ee, ev_str, "mama");
	ee_remove_listener(ee, ev_str, on_hello);
	ee_destroy(ee);
return 0;	
}
