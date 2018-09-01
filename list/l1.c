#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

#define green "\x1b[32m"
#define yellow "\x1b[33m"
#define red "\x1b[31m"
#define rst "\x1b[0m"

typedef struct {
int bob;
}ee_handler_t;

typedef struct ee__event_t
{
const char*name;
ee_handler_t *hah;
LIST_ENTRY(ee__event_t) hans;
} ee__event_tt;
typedef LIST_HEAD(head_ev, ee__event_t) headev_t;

typedef struct node
{
int c;
struct ee__event_t*events;
LIST_ENTRY(node) nodes;
} node_t;

typedef LIST_HEAD(head_s, node) head_t;


struct ee__event_t*ee__event_new(const char*name){
struct ee__event_t*event;
event=malloc(sizeof(struct ee__event_t));
if(event==NULL)return NULL;
event->name=NULL;
event->name=strdup(name);
event->hah=NULL;
return event;	
}


struct ee__event_t*ee__find(head_t*head, const char * x){
struct node*e=NULL;
LIST_FOREACH(e, head, nodes){
printf("finding: %p\n",(void*)e->events);	

if(!strcmp(e->events->name, x)){printf(green "ok, found it!\n" rst);
return e->events;
}
}
return NULL;	
}


void ee__on(headev_t*evi,head_t*head,const char*name){
	struct ee__event_t*event;
	ee_handler_t*handler;
	event = ee__find(head, name);
	
	if(event==NULL){
		printf("it looks like event is NULL in ee__on\n");
	event=ee__event_new(name);
	
	struct node*e=malloc(sizeof(struct node));
	if(e==NULL){
		fprintf(stderr,red "malloc failed in ee_on\n" rst);
		free((void*)event->name);
		event->name=NULL;
		free(event);
		exit(EXIT_FAILURE);
		}
		e->c=4;
		e->events=NULL;
		e->events=event;
		printf("mem of e: %p\n",(void*)e);
		printf("mem of e->events: %p\n", (void*)e->events);
		LIST_INSERT_HEAD(head,e,nodes);
		e=NULL;	
	}
	
	handler=malloc(sizeof*handler);
	handler->bob=200;
	event->hah=handler;
	printf("mem of event?: %p\n",(void*)event);
	LIST_INSERT_HEAD(evi,event, hans);
	}

int i=0;


void print_queue(headev_t*evi){
printf("entering print_queue\n");
struct ee__event_t*e=NULL;
//struct node*np_tmp=NULL;
LIST_FOREACH(e, evi, hans){
printf("e->hah->bob: %d\n",e->hah->bob);
}	
}

void free_ev(headev_t*evi){
printf("entering free_ev()\n");	
struct ee__event_t*e=NULL;
struct ee__event_t*next=NULL;
//while(!LIST_EMPTY(evi))
LIST_FOREACH_SAFE(e, evi, hans, next)
{

//e=LIST_FIRST(evi);

if(e->name !=NULL){
printf("e->name: %s\n",e->name);
free((char*)e->name);
//e->name=NULL;
}
//printf("e->hah->bob: %d\n",e->hah->bob);
free(e->hah);
e->hah=NULL;
printf("mem of event: %p\n",(void*)e);
LIST_REMOVE(e, hans);

if(e !=NULL)//free(e);
e=NULL;
}

}
 
void free_queue(head_t * head){
printf("\nentering free_queue\n");
struct node*e=NULL;
while(!LIST_EMPTY(head)){
i++;
e=LIST_FIRST(head);
LIST_REMOVE(e,nodes);
if(e->events==NULL){printf("events is null\n");}else{printf("head->events is not null: %p\n",(void*)e->events);
	//free(e->events);
	//e->events=NULL;
	}
printf("mem of e: %p\n",(void*)e);
free(e);
e=NULL;
}
}
	
int main(){
	
	head_t head;
	//struct node*fuck;
	headev_t evi;
	LIST_INIT(&head);
	LIST_INIT(&evi);
	ee__on(&evi, &head, "suka");
	
	ee__on(&evi, &head, "suka");
	//print_queue(&evi);
	
	free_ev(&evi);
	free_queue(&head);
	
	/*
	fill_queue(&head, "looser");
	print_queue(&head);
	printf("\n");
	fuck=find(&head, 5);
	printf("fuck->c found?: %d\n",fuck->c);
	free_queue(&head);
	printf("after free: %d\n",i);
	print_queue(&head);
	 */
	 printf("*** bye! ***\n");
	return 0;	
}
