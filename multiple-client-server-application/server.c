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

#define MAXMESSAGE 300
#define MAX_PENDING_CONNECTION_QUEUE 10

void validate_args(int argc, char **argv){
    if (argc != 2) {
        printf("Error: ./program <PortNumber>\n");
        exit(EXIT_FAILURE);
    }
}

int create_new_socket(){
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if ( sockfd < 0) {
     printf("Error in method create_new_socket.\n");
     exit(EXIT_FAILURE);
  }

  return sockfd;
}

void initialize_server_address(struct sockaddr_in * server_address, int portNumber){
  bzero(server_address, sizeof(* server_address));
  server_address->sin_family      = AF_INET;
  server_address->sin_addr.s_addr = htonl(INADDR_ANY);
  server_address->sin_port        = htons(portNumber);
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
  socklen_t length;
  struct sockaddr_in peer_address;
  char str[INET_ADDRSTRLEN];

  length = sizeof(peer_address);
  int connfd = accept(sockfd, (struct sockaddr *) &peer_address, &length);

  if(connfd < 0){
    printf("Erro to accept connection.\n");
    exit(EXIT_FAILURE);
  }

  inet_ntop(AF_INET, &(peer_address.sin_addr), str, INET_ADDRSTRLEN);
  printf("New connection - IP: %s PortNumber:%d\n", str, ntohs(peer_address.sin_port));

  return connfd;
}

void ask_for_command(int connfd){
  char message_to_client[] = "Enter a command: ";
  write(connfd, message_to_client, strlen(message_to_client) + 1);
}

void read_execute_command(int connfd){
  char message_from_client[MAXMESSAGE], formated_message_server[1024];

  bzero( message_from_client, MAXMESSAGE);
  bzero( formated_message_server, 1024);

  if(read(connfd, message_from_client, MAXMESSAGE) > 0) {
    //write in server
    strcpy(formated_message_server, "Command received: ");
    strcat(formated_message_server, message_from_client);
    printf("%s", formated_message_server);

    //write in client
    write(connfd, message_from_client, strlen(message_from_client) + 1);

    system(message_from_client);
  }
}

void handle_client(int connfd){

    ask_for_command(connfd);
    read_execute_command(connfd);
}

int main(int argc, char **argv){
  int sockfd, connfd;
  struct sockaddr_in server_address;
  pid_t process_id;

  printf("Starting server...\n");

  validate_args(argc, argv);
  sockfd = create_new_socket();
  initialize_server_address(&server_address, atoi(argv[1]));
  bind_name_to_socket((struct sockaddr *) &server_address, sockfd);
  listen_for_connections(sockfd);

  printf("Waiting for connections...\n");

  for(;;){
    connfd = accept_connection(sockfd);

    process_id = fork();
    //if is child
    if(process_id == 0) {
       close(sockfd);
       while(1){
          handle_client(connfd);
       }
       close(connfd);
       exit(0);
    }

    close(connfd);
  }

  return 0;
}
