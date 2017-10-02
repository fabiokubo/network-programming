#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define PORTNUMBER 6000

void validate_args(int argc, char **argv){
    if (argc != 2) {
        printf("Error: ./program <IPAddress> \n");
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

void init_server_address(char * server_ip, struct sockaddr_in * server_address){

    //Initialize server_address, a struct containing server address informations
    bzero(server_address, sizeof(*server_address));
    //Set config values for server_address
    server_address->sin_family = AF_INET;
    server_address->sin_port   = htons(PORTNUMBER);

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

void send_command_to_server(int sockfd){
    char message_to_server[300];

    /*Read at most 100 chars from console into 'str' */
    read(0, message_to_server, 300);

    //send to server
    write(sockfd, message_to_server, strlen(message_to_server) + 1);
}

void print_from_server(int sockfd) {
  char message_from_server[300];

  read(sockfd, message_from_server, 300);

  printf("%s\n", message_from_server);
}

int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in server_address;

    validate_args(argc, argv);
    sockfd = create_new_socket();
    init_server_address(argv[1], &server_address);
    start_connection(sockfd, (struct sockaddr *) &server_address);

    //forever
    for(;;){
        print_from_server(sockfd);
        send_command_to_server(sockfd);
        print_from_server(sockfd);

        exit(EXIT_FAILURE);
    }

    return 0;
}
