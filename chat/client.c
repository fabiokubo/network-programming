#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#define MAXMESSAGE 1000

void validate_args(int argc, char **argv){
    if (argc != 3) {
        printf("Error: ./program <IPAddress> <PortNumber>\n");
        exit(EXIT_FAILURE);
    }
}

int create_new_socket(){
    //Creates a new socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
       printf("Failed to create new socket.");
       exit(EXIT_FAILURE);
    }
    return sockfd;
}

void init_server_address(char * server_ip, struct sockaddr_in * server_address, int portNumber){

    //Initialize server_address, a struct containing server address informations
    bzero(server_address, sizeof(*server_address));
    //Set config values for server_address
    server_address->sin_family = AF_INET;
    server_address->sin_port   = htons(portNumber);

    //Converts server address IP from string to binary and saves in server_address
    if (inet_pton(AF_INET, server_ip, &(server_address->sin_addr) ) <= 0) {
        printf("Error to convert server ip from text to binary.\n");
        exit(EXIT_FAILURE);
    }
}

void start_connection(int sockfd, struct sockaddr *servaddr){
    //Start connection with server using sockfd socket
    if (connect(sockfd,  servaddr, sizeof(*servaddr)) < 0) {
       printf("Error connecting with server.\n");
       exit(EXIT_FAILURE);
    }
}

void close_client(int sockfd) {
  printf("Disconnecting from server...\n");
  close(sockfd);
  printf("See ya!\n");
  exit(EXIT_SUCCESS);
}

int isExitMessage(char * message_to_server) {
  //fgets function adds \n in string, if is exit command, exit from program
  return strncmp(message_to_server, "exit\n", MAXMESSAGE) == 0;
}

void send_command_to_server(int sockfd){
    char message_to_server[MAXMESSAGE], message_from_server[MAXMESSAGE];
    int n, is_std_eof;
    fd_set rset;

    printf("Welcome!\n");

    is_std_eof = 0;

    FD_ZERO(&rset);
    //forever
    for ( ; ; ) {

      if(is_std_eof == 0)
        FD_SET(fileno(stdin), &rset);

      FD_SET(sockfd, &rset);

      //set first parameter to the highest fd + 1
      select(sockfd + 1, &rset, NULL, NULL, NULL);

      if (FD_ISSET(sockfd, &rset)) {	/* socket is readable */
        if ( (n = read(sockfd, message_to_server, MAXMESSAGE)) == 0) {
          if (is_std_eof == 1)
            return;		/* normal termination */
          else {
              printf("Error: could not read socket.\n");
              exit(EXIT_FAILURE);
          }
        }

        write(fileno(stdout), message_to_server, n);
      }

      if (FD_ISSET(fileno(stdin), &rset)) {  /* input is readable */

        //last message
        if ( (n = read(fileno(stdin), message_from_server, MAXMESSAGE)) == 0) {
          is_std_eof = 1;
          shutdown(sockfd, SHUT_WR);	/* send FIN */
          FD_CLR(fileno(stdin), &rset);
          continue;
        }

        write(sockfd, message_from_server, n);

      }
    }
}

void print_from_server(int sockfd) {
  char message_from_server[MAXMESSAGE];

  while(read(sockfd, message_from_server, MAXMESSAGE) > 0) {
    printf("%s\n", message_from_server);
  }
}

void print_connection_info(char **argv, int sockfd){
    struct sockaddr_in client_info;
    char str[INET_ADDRSTRLEN];
    socklen_t sa_len;

    sa_len = sizeof(client_info);
    getsockname(sockfd,  (struct sockaddr *) &client_info, &sa_len);
    inet_ntop(AF_INET, &(client_info.sin_addr), str, INET_ADDRSTRLEN);
    printf("Initiating connection IP Address %s and Port %s.\n", argv[1], argv[2]);
    printf("Client application with IP Address %s and Port %d.\n", str, ntohs(client_info.sin_port));
}


int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in server_address;

    validate_args(argc, argv);
    sockfd = create_new_socket();
    init_server_address(argv[1], &server_address, atoi(argv[2]));
    start_connection(sockfd, (struct sockaddr *) &server_address);
    print_connection_info(argv, sockfd);

    send_command_to_server(sockfd);

    return 0;
}
