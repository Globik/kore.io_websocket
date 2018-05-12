
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>

static uint32_t rand32(uint32_t begin,uint32_t end){
uint32_t range=(end-begin)+1;
	uint32_t limit=((uint64_t)RAND_MAX+1)+(((uint64_t)RAND_MAX+1)%range);
	uint32_t randVal=rand();
	while(randVal >=limit) randVal=rand();
	return (randVal%range)+begin;
}
uint32_t random_u32(){
	uint32_t a=rand32(10000000,99999999);
	//printf("a: %"PRIu32"\n",a);
return a;
}