#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORTNUMBER 6000
#define MAX_PENDING_CONNECTION_QUEUE 10


int create_new_socket(){
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);

  if ( listenfd < 0) {
     printf("Error in method create_new_socket.");
     exit(EXIT_FAILURE);
  }

  return listenfd;
}

void initialize_server_address(struct sockaddr_in * server_address){
  bzero(server_address, sizeof(* server_address));
  server_address->sin_family      = AF_INET;
  server_address->sin_addr.s_addr = htonl(INADDR_ANY);
  server_address->sin_port        = htons(PORTNUMBER);
}

void bind_name_to_socket(struct sockaddr * servaddr, int listenfd){
  int bind_result = bind(listenfd, servaddr, sizeof(* servaddr));

  if (bind_result < 0) {
     printf("Error in method bind_name_to_socket.");
     exit(EXIT_FAILURE);
  }
}

void listen_for_connections(int listenfd){
  int listen_result = listen(listenfd, MAX_PENDING_CONNECTION_QUEUE);
  if ( listen_result < 0) {
     printf("Error in method listen_for_connections");
     exit(EXIT_FAILURE);
  }
}

int main(){
  int listenfd, connfd;
  struct sockaddr_in server_address;
  pid_t process_id;

  listenfd = create_new_socket();
  initialize_server_address(&server_address);
  bind_name_to_socket((struct sockaddr *) &server_address, listenfd);
  listen_for_connections(listenfd);

  while(1){
    if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
       perror("accept");
       exit(1);
    }

    //fork process
    process_id = fork();

    //if is child
    if(process_id == 0) {
       close(listenfd);
       //process_request();

       write(connfd, "teste", 6);

       close(connfd);
       exit(0);
    }

    close(connfd);
  }

  return 0;
}
