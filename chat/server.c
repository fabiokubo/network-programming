#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFLEN 512
#define MAX_CLIENTS 10

typedef struct Client {
  int portNumber;
  char iPAddress[20];
  char nickname[50];
} Client;

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
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

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

int main(int argc, char **argv){
  struct sockaddr_in server_address, peer_address;
  socklen_t slen;
  int sockfd, recv_len;
  char buf[BUFLEN];

  //array with all known clients
  Client clients[MAX_CLIENTS];

  //quantity of known clients
  int n_clients = 0;


  printf("Starting server...\n");

  validate_args(argc, argv);
  sockfd = create_new_socket();
  initialize_server_address(&server_address, atoi(argv[1]));
  bind_name_to_socket((struct sockaddr *) &server_address, sockfd);

  //loops forever
  for(;;){
      printf("Waiting for data...");
      fflush(stdout);

      //try to receive some data, this is a blocking call
      slen = sizeof(peer_address);
      if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &peer_address, &slen)) == -1){
        printf("Error: receiving message.\n");
        exit(EXIT_FAILURE);
      }

      //print details of the client/peer and the data received
      printf("Received packet from %s:%d\n", inet_ntoa(peer_address.sin_addr), ntohs(peer_address.sin_port));
      printf("Data: %s\n" , buf);

      //now reply the client with the same data
      if (sendto(sockfd, buf, recv_len, 0, (struct sockaddr*) &peer_address, slen) == -1){
        printf("Error: sending message.\n");
        exit(EXIT_FAILURE);
      }
  }

  return 0;
}
