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

#define MAXMESSAGE 1000
#define MAX_PENDING_CONNECTION_QUEUE 10

void log_connection_file(struct sockaddr_in * peer_address) {
  FILE *fp;
  char str[INET_ADDRSTRLEN];

  fp = fopen ("log.txt", "a");

  inet_ntop(AF_INET, &(peer_address->sin_addr), str, INET_ADDRSTRLEN);
  printf("%s:%d connected. Unix timestamp: %lu.\n", str, ntohs(peer_address->sin_port), (unsigned long)time(NULL));
  fprintf(fp, "%s:%d connected. Unix timestamp: %lu.\n", str, ntohs(peer_address->sin_port), (unsigned long)time(NULL));

  fclose(fp);
}

void log_disconnection_file(struct sockaddr_in * peer_address) {
  FILE *fp;
  char str[INET_ADDRSTRLEN];

  fp = fopen ("log.txt", "a");

  inet_ntop(AF_INET, &(peer_address->sin_addr), str, INET_ADDRSTRLEN);
  printf("%s:%d disconnected. Unix timestamp: %lu.\n", str, ntohs(peer_address->sin_port), (unsigned long)time(NULL));
  fprintf(fp, "%s:%d disconnected. Unix timestamp: %lu.\n", str, ntohs(peer_address->sin_port), (unsigned long)time(NULL));

  fclose(fp);
}

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

int isExitMessage(char * message_to_server) {
  //fgets function adds \n in string, if is exit command, exit from program
  return strncmp(message_to_server, "CLIENT_EXIT_123", MAXMESSAGE) == 0;
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

int accept_connection(int sockfd, struct sockaddr_in * peer_address){
  socklen_t length;
  char str[INET_ADDRSTRLEN];

  length = sizeof(peer_address);
  int connfd = accept(sockfd, (struct sockaddr *) peer_address, &length);

  if(connfd < 0){
    printf("Error to accept connection.\n");
    exit(EXIT_FAILURE);
  }

  inet_ntop(AF_INET, &(peer_address->sin_addr), str, INET_ADDRSTRLEN);
  log_connection_file(peer_address);

  return connfd;
}

void execute_command(int connfd, char * command) {
  FILE *fp;
  char output[MAXMESSAGE], outputAux[MAXMESSAGE];

  memset(output, 0, MAXMESSAGE);
  memset(outputAux, 0, MAXMESSAGE);

  /* Open the command for reading. */
  fp = popen(command, "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(EXIT_FAILURE);
  }

  while ( fgets(outputAux, sizeof(outputAux), fp) != NULL) {
    strcat(output, outputAux);
    memset(outputAux, 0, MAXMESSAGE);
  }

  write(connfd, output, strlen(output));
  printf("%s\n", output);
  fflush(stdout);

  /* close */
  pclose(fp);
}

void read_execute_commands(int connfd, struct sockaddr_in * peer_address){
  char message_from_client[MAXMESSAGE];
  char str[INET_ADDRSTRLEN];

  for(;;) {
    memset(message_from_client, 0, MAXMESSAGE);

    if(read(connfd, message_from_client, MAXMESSAGE) > 0) {
      if(!isExitMessage(message_from_client)) {

        //print client IP and Port
        inet_ntop(AF_INET, &(peer_address->sin_addr), str, INET_ADDRSTRLEN);
        printf(">>IP:%s PORT: %d COMMAND:%s", str, ntohs(peer_address->sin_port), message_from_client);
        execute_command(connfd, message_from_client);
      }
      else {
        log_disconnection_file(peer_address);
        close(connfd);
      }
    }
  }
}

int main(int argc, char **argv){
  int sockfd, connfd;
  struct sockaddr_in server_address, peer_address;;
  pid_t process_id;

  printf("Starting server...\n");

  validate_args(argc, argv);
  sockfd = create_new_socket();
  initialize_server_address(&server_address, atoi(argv[1]));
  bind_name_to_socket((struct sockaddr *) &server_address, sockfd);
  listen_for_connections(sockfd);

  printf("Waiting for connections...\n");

  for(;;){
    connfd = accept_connection(sockfd, &peer_address);

    process_id = fork();

    if(process_id != 0) {
        close(connfd);
    }
    else{ //if is child
      close(sockfd);
      read_execute_commands(connfd, &peer_address);
      exit(0);
    }
  }

  return 0;
}
