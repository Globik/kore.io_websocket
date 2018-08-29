// https://blog.taborkelly.net/programming/c/2016/01/09/sys-queue-example.html
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h" //queue taken from freebsd 10

typedef struct node
{
	char c;
	TAILQ_ENTRY(node) nodes;
} node_t;
typedef TAILQ_HEAD(head_s, node) head_t;

void fill_queue(head_t*head,const char*string){
int c=0;
for(c=0;c<strlen(string);++c){
	struct node*e=malloc(sizeof(struct node));
	if(e==NULL){
		fprintf(stderr,"malloc failed");
		exit(EXIT_FAILURE);
		}
		e->c=string[c];
		TAILQ_INSERT_TAIL(head,e,nodes);
		e=NULL;
	}	
}

void free_queue(head_t * head){
	struct node*e=NULL;
	while(!TAILQ_EMPTY(head)){
		e=TAILQ_FIRST(head);
		TAILQ_REMOVE(head,e,nodes);
		free(e);
		e=NULL;
		}
	}
	
void print_queue(head_t*head){
struct node*e=NULL;
TAILQ_FOREACH(e,head,nodes){
printf("%c",e->c);	

}	
}

void remove_vowels(head_t * head){
	struct node * e=NULL;
	struct node * next=NULL;
	TAILQ_FOREACH_SAFE(e,head,nodes,next)
	{
		if(e->c == 'a' || e->c=='A' || e->c=='e' || e->c=='E' ||
		e->c=='i' || e->c=='I' || e->c=='o' || e->c=='O' ||
		e->c =='u' || e->c=='U'){
			TAILQ_REMOVE(head,e,nodes);
			free(e);
			e=NULL;
			}
	}
}

void print_queue_backwards(head_t*head){
struct node*e=NULL;
TAILQ_FOREACH_REVERSE(e,head,head_s,nodes){printf("%c",e->c);}	
}

int main(){
	head_t head;
	TAILQ_INIT(&head);
	fill_queue(&head,"hello world");
	printf("\n");
	print_queue(&head);
	printf("\n");
	print_queue_backwards(&head);
	printf("\n");
	remove_vowels(&head);
	print_queue(&head);
	printf("\n");
	free_queue(&head);
	print_queue(&head);
	
	
	return 0;
}
