#include "plugin.h"
j_plugin_res*j_plugin_res_new(j_plugin_res_type type,const char*text){
	g_print("creating plugin result...\n");
j_plugin_res *result=g_malloc(sizeof(j_plugin_res));
	result->type=type;
	result->text=text;
	return result;
}
void j_plugin_res_destroy(j_plugin_res *result){
result->text=NULL;
	g_free(result);
	g_print("j_plugin_res_destroy\n");
}

