#include <kore/kore.h>
int main(){
struct kore_buf*b;
b=kore_buf_alloc(128);
kore_free(b);
return 0;	
}
