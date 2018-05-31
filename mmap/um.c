#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "seq/message.h"
#define socket_name "/home/globik/fuck"
#define buffer_size 512
 char* client_name="alikon_chelikon";
peer_t server;

int connect_server(peer_t *server)
{
struct sockaddr_un addr;
  int ret;
server->socket=socket(AF_UNIX, SOCK_SEQPACKET/* | O_NONBLOCK*/, 0);
if(server->socket<0){
perror("socket()");
return -1;
}
memset(&addr,0,sizeof(struct sockaddr_un));
addr.sun_family=AF_UNIX;
strncpy(addr.sun_path, socket_name,sizeof(addr.sun_path)-1);
server->addres = addr;
ret=connect(server->socket,(const struct sockaddr*)&addr,sizeof(struct sockaddr_un));
if(ret !=0){perror("err connect");return -1;}
printf("Connected to unix socket : %s\n", socket_name);
	/*
	int flag=fcntl(server->socket, F_GETFL,0);
	if(flag<0){printf("some flag1 err\n");}
	flag |=O_NONBLOCK;
	if(fcntl(server->socket, F_SETFL, flag)<0){printf("some flag2 err\n");}
	if(ret==EINPROGRESS){printf("ret einprogress\n");}else{printf("kuku : %d\n",ret);}
	*/
return 0;
}

int build_fd_sets(peer_t *server, fd_set *read_fds, fd_set *write_fds, fd_set *except_fds)
{
  FD_ZERO(read_fds);
  FD_SET(STDIN_FILENO, read_fds);
  FD_SET(server->socket, read_fds);
  
  FD_ZERO(write_fds);
  // there is smth to send, set up write_fd for server socket
	printf("SERVER->SEND_BUFFER.CURRENT => %d\n",server->send_buffer.current);
  if (server->send_buffer.current > 0){ 
	  printf("there is smth to send, set up write_fd for server socket!!!\n");
	  FD_SET(server->socket, write_fds);
  }
  
  FD_ZERO(except_fds);
  FD_SET(STDIN_FILENO, except_fds);
  FD_SET(server->socket, except_fds);
  printf("LEAVE BUILD FDS\n");
  return 0;
}

int handle_read_from_stdin(peer_t *server, char *client_name)
{
  char read_buffer[DATA_MAXSIZE]; // buffer for stdin
  if (read_from_stdin(read_buffer, DATA_MAXSIZE) != 0)
    return -1;
  
  // Create new message and enqueue it.
  message_t new_message;
  prepare_message(client_name, read_buffer, &new_message);
  print_message(&new_message);
  
  if (peer_add_to_send(server, &new_message) != 0) {
    printf("Send buffer is overflowed, we lost this message!\n");
    return 0;
  }
  printf("New message to send was enqueued right now.\n");
  
  return 0;
}
void shutdown_properly(int code)
{
  delete_peer(&server);
  printf("Shutdown client properly.\n");
  exit(code);
}

int handle_received_message(message_t *message)
{
  printf("Received message from server.\n");
  print_message(message);
  return 0;
}

 
int main(){
	 create_peer(&server);
  if (connect_server(&server) != 0)
    shutdown_properly(EXIT_FAILURE);
  
  /* Set nonblock for stdin. */
  int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
  flag |= O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, flag);
  
  fd_set read_fds;
  fd_set write_fds;
  fd_set except_fds;
  
  printf("Waiting for server message or stdin input. Please, type text to send:\n");
  
  // server socket always will be greater then STDIN_FILENO
  int maxfd = server.socket;
  
	while(1){
		printf("entering while\n");
		// Select() updates fd_set's, so we need to build fd_set's before each select()call.
    build_fd_sets(&server, &read_fds, &write_fds, &except_fds);
    printf("BEFORE ACTIVITY\n");
    int activity = select(maxfd + 1, &read_fds, &write_fds, &except_fds, NULL);
	  
    printf("ACTIVITY: %d\n",activity);
		
	
	switch (activity) {
      case -1:
        perror("select()");
        shutdown_properly(EXIT_FAILURE);

      case 0:
        // you should never get here
        printf("select() returns 0.\n");
        shutdown_properly(EXIT_FAILURE);

      default:
        /* All fd_set's should be checked. */
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
			printf("STDIN READ_FDS!!!!!!!!\n");
          if (handle_read_from_stdin(&server, client_name) != 0)
            shutdown_properly(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &except_fds)) {
          printf("except_fds for stdin.\n");
          shutdown_properly(EXIT_FAILURE);
        }

        if (FD_ISSET(server.socket, &read_fds)) {
			 printf("SERVER READ_FDS!!!!!!!!!!!\n");
          if (receive_from_peer(&server, &handle_received_message) != 0)
            shutdown_properly(EXIT_FAILURE);
        }

        if (FD_ISSET(server.socket, &write_fds)) {
			printf("SERVER WRITE_FDS!!!!!!!!!!!!!!!!\n");
          if (send_to_peer(&server) != 0)
            shutdown_properly(EXIT_FAILURE);
        }

        if (FD_ISSET(server.socket, &except_fds)) {
          printf("except_fds for server.\n");
          shutdown_properly(EXIT_FAILURE);
        }
    }
    
    printf("And we are \n");
		}
  
  return 0;
}