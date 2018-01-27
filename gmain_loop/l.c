#include <glib.h>
GMainLoop*loop;
gint count=10;

gboolean cb(gpointer arg){
g_print("hello world\n");
g_message("msg");
	//fflush();
if(--count==0){
g_print("g main loop quit\n");
g_main_loop_quit(loop);
return FALSE;
}
return TRUE;
}
int main(int argc,char*argv[]){
//if(g_thread_supported()==0)
	//g_thread_init(NULL);
	g_print("g main loop new\n");
	loop=g_main_loop_new(NULL,FALSE);
	g_timeout_add(1000,cb,NULL);
	g_print("g main loop run\n");
	g_main_loop_run(loop);
	g_print("g main loop unref\n");
	g_main_loop_unref(loop);
	return 0;
}
/*

gcc -o l l.c `pkg-config --cflags --libs glib-2.0`

*/