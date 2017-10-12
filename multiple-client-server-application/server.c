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

void log_connection_file(struct sockaddr_in * peer_address) {
  FILE *fp;
  char str[INET_ADDRSTRLEN];

  fp = fopen ("log.txt", "a");

  inet_ntop(AF_INET, &(peer_address->sin_addr), str, INET_ADDRSTRLEN);
  fprintf(fp, "%s:%d connected. Unix timestamp: %lu.\n", str, ntohs(peer_address->sin_port), (unsigned long)time(NULL));

  fclose(fp);
}

void log_disconnection_file(struct sockaddr_in * peer_address) {
  FILE *fp;
  char str[INET_ADDRSTRLEN];

  fp = fopen ("log.txt", "a");

  inet_ntop(AF_INET, &(peer_address->sin_addr), str, INET_ADDRSTRLEN);
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
    printf("Erro to accept connection.\n");
    exit(EXIT_FAILURE);
  }

  inet_ntop(AF_INET, &(peer_address->sin_addr), str, INET_ADDRSTRLEN);
  printf("New connection - IP: %s PortNumber:%d\n", str, ntohs(peer_address->sin_port));

  log_connection_file(peer_address);

  return connfd;
}

void ask_for_command(int connfd){
  char message_to_client[] = "Enter a command: ";
  write(connfd, message_to_client, strlen(message_to_client) + 1);
}

void execute_command(int connfd, char * command) {
  FILE *fp;
  char output[1035];

  /* Open the command for reading. */
  fp = popen(command, "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(EXIT_FAILURE);
  }

  /* Read the output a line at a time - output it. */
  while (fgets(output, sizeof(output)-1, fp) != NULL) {
    write(connfd, output, strlen(output) + 1);
  }

  /* close */
  pclose(fp);
}

void read_execute_command(int connfd, struct sockaddr_in * peer_address){
  char message_from_client[MAXMESSAGE];
  char str[INET_ADDRSTRLEN];

  bzero( message_from_client, MAXMESSAGE);

  if(read(connfd, message_from_client, MAXMESSAGE) > 0) {

    inet_ntop(AF_INET, &(peer_address->sin_addr), str, INET_ADDRSTRLEN);

    printf("%s:%d sent: %s\n", str, ntohs(peer_address->sin_port), message_from_client);

    //write in client
    write(connfd, message_from_client, strlen(message_from_client) + 1);

    system(message_from_client);
  }
}

void handle_client(int connfd, struct sockaddr_in * peer_address){

    ask_for_command(connfd);
    read_execute_command(connfd, peer_address);
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
    //if is child
    if(process_id == 0) {
       close(sockfd);
       while(1){
          handle_client(connfd, &peer_address);
       }
       close(connfd);
       exit(0);
    }

    close(connfd);
  }

  return 0;
}
