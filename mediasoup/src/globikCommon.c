#include "globikCommon.h"

json_t *load_json_buf(const char*text,size_t buflen){
json_t *root;
json_error_t error;
root=json_loadb(text,buflen,0,&error);
if(root){
return root;
}else{
printf(red "json error on line %d: %s\n" rst, error.line, error.text);
return (json_t*)0;
}
}
json_t *load_json_str(const char*text){
json_t *root;
json_error_t error;
root=json_loads(text,0,&error);
if(root){
return root;
}else{
printf(red "json error on line %d: %s\n" rst, error.line, error.text);
return (json_t*)0;
}
}
