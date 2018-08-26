// https://tia.mat.br/posts/2018/02/01/more_on_string_switch_in_c.html
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>
#include <memory.h>
//static inline uint32_t string_to_uint32(const char*);
static inline uint32_t  string_to_uint32(const char*s){
	uint32_t v;
	memcpy(&v,s,sizeof(v));
	printf("within string_to_uint32: %s\n",s);
	printf("within is %" PRIu32 "\n",v);
	return v;
}
//int suka=0;
#define STRING_SWITCH(s) switch (string_to_uint32(s))
// for a big endian??
//#define MCC(a,b,c,d) ((a) << 24 | (b) << 16 | (c) << 8 | (d))
//for a little endian???
#define MCC(a,b,c,d) ((a) | (b) << 8 | (c) << 16 | (d) << 24)

#if __BYTE_ORDER__== __ORDER_LITTLE_ENDIAN__
int suka=2;
#endif

#if __BYTE_ORDER__== __ORDER_BIG_ENDIAN__
int suka=4;
#endif

#if defined(__ORDER_LITTLE_ENDIAN__)
int buka=9;
#endif

enum{
	GET=MCC('G','E','T','i'),
	POST=MCC('P','O','S','T'),
	DELETE=MCC('D','E','L','E'),
	HEAD=MCC('H','E','A','D')
};
	

int main(){
const char*f="HEAD";
printf("what is %" PRIu32 "\n",GET);
STRING_SWITCH(f){
	
case GET:
printf("is get?\n");
break;
case POST:
printf("is post?\n");
break;
case DELETE:
printf("is delete?\n");
break;
case HEAD:
printf("is head?\n");
break;
default:
printf("UNKNOWN METHOD\n");	
}
printf("suka: %d %d\n",suka,buka);
printf("***bye!***\n");
return 0;	
}
