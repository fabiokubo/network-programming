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

#define PORTNUMBER 3000
#define MAX_PENDING_CONNECTION_QUEUE 10


int create_new_socket(){
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if ( sockfd < 0) {
     printf("Error in method create_new_socket.\n");
     exit(EXIT_FAILURE);
  }

  return sockfd;
}

void initialize_server_address(struct sockaddr_in * server_address){
  bzero(server_address, sizeof(* server_address));
  server_address->sin_family      = AF_INET;
  server_address->sin_addr.s_addr = htonl(INADDR_ANY);
  server_address->sin_port        = htons(PORTNUMBER);
}

void bind_name_to_socket(struct sockaddr * servaddr, int sockfd){
  int bind_result = bind(sockfd, servaddr, sizeof(* servaddr));

  if (bind_result < 0) {
     printf("Error in method bind_name_to_socket.\n");
     exit(EXIT_FAILURE);
  }
}

void listen_for_connections(int sockfd){
  int listen_result = listen(sockfd, MAX_PENDING_CONNECTION_QUEUE);
  if ( listen_result < 0) {
     printf("Error in method listen_for_connections.\n");
     exit(EXIT_FAILURE);
  }
}

int accept_connection(int sockfd){
  int connfd = accept(sockfd, (struct sockaddr *) NULL, NULL);

  if(connfd < 0){
    printf("Erro to accept connection.\n");
    exit(EXIT_FAILURE);
  }

  return connfd;
}

void ask_for_command(int connfd){
  char message_to_client[300] = "Enter a command: ";
  write(connfd, message_to_client, strlen(message_to_client) + 1);
}

void read_command(int connfd){
  char message_from_client[300], formated_message_server[1024]
  , formated_message_client[1024];

  read(connfd, message_from_client, 300);

  strcpy(formated_message_server, "Command received: ");
  strcat(formated_message_server, message_from_client);
  strcat(formated_message_server, "\n");

  //write in server
  printf("%s\n", formated_message_server);
  
  strcpy(formated_message_client, "Server received: ");
  strcat(formated_message_client, message_from_client);
  strcat(formated_message_client, "\n");

  //write in client
  write(connfd, formated_message_client, strlen(formated_message_client) + 1);
}

int main(){
  int sockfd, connfd, n_connections;
  struct sockaddr_in server_address;
  pid_t process_id;

  printf("Starting server...\n");

  sockfd = create_new_socket();
  initialize_server_address(&server_address);
  bind_name_to_socket((struct sockaddr *) &server_address, sockfd);
  listen_for_connections(sockfd);

  printf("Waiting for connections...\n");
  n_connections = 0;

  while(1){
    connfd = accept_connection(sockfd);

    //increment the quantity of connected clients
    n_connections++;
    printf("Client %d connected\n", n_connections);

    //fork process
    process_id = fork();

    //if is child
    if(process_id == 0) {
       close(sockfd);
       ask_for_command(connfd);
       read_command(connfd);
       //execute_command();
       //send_output();

       close(connfd);
       exit(0);
    }

    close(connfd);
  }

  return 0;
}
