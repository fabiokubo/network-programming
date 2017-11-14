#include "utils.h"

void validate_parameters(int argc, char **argv){
    if (argc != 3) {
        printf("Error: ./client <IPAddress> <PorNumber>\n");
        exit(EXIT_FAILURE);
    }
}

int create_new_socket(){
    //Creates a new socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
       printf("Failed to create new socket.");
       exit(EXIT_FAILURE);
    }
    return sockfd;
}

void initialize_server_address(char * server_ip, struct sockaddr_in * server_address, int portNumber){
  bzero(server_address, sizeof(* server_address));
  server_address->sin_family      = AF_INET;
  server_address->sin_port        = htons(portNumber);

  //Converts server address IP from string to binary and saves in server_address
  if (inet_pton(AF_INET, server_ip, &(server_address->sin_addr) ) <= 0) {
      printf("Error to convert server ip from text to binary.\n");
      exit(EXIT_FAILURE);
  }
}

void die(char *s){
    perror(s);
    exit(1);
}

void sentMessageToServer(int sockfd, char * message, struct sockaddr * server_address, socklen_t slen){
  if (sendto(sockfd, message, strlen(message) , 0 , server_address, slen)==-1){
    die("sendto()");
    //printf("Error: sendto() method.\n");
    //exit(EXIT_FAILURE);
  }
}

void receiveMessageFromServer(int sockfd, char * buf, struct sockaddr * server_address, socklen_t * slen){
  if (recvfrom(sockfd, buf, BUFLEN, 0, server_address, slen) == -1){
    printf("Error: recvfrom() method.\n");
    exit(EXIT_FAILURE);
  }
}

void sentRequestMessage(int sockfd, char * nickname, struct sockaddr * server_address, socklen_t slen){

  char message[BUFLEN];

  //first message
  message[0] = REGISTER_USER;

  //copying nickname into first message
  strcpy(&message[1], nickname);

  sentMessageToServer(sockfd, message, server_address, slen);
}

int main(int argc, char **argv)
{
    struct sockaddr_in server_address;
    socklen_t slen=sizeof(server_address);
    int sockfd;
    char buf[BUFLEN], message[BUFLEN], nickname[50];

    validate_parameters(argc, argv);

    sockfd = create_new_socket();
    initialize_server_address(argv[1], &server_address, atoi(argv[2]));

    printf("Enter your nickname:\n");
    fgets(nickname , 50 , stdin);

    printf("Trying to connect with server...");
    sentRequestMessage(sockfd, nickname, (struct sockaddr *) &server_address, slen);

    for(;;){

        //send the message
        //sentMessageToServer(sockfd, message, (struct sockaddr *) &server_address, slen);

        memset(buf,'\0', BUFLEN);
        receiveMessageFromServer(sockfd, buf, (struct sockaddr *) &server_address, &slen);

        puts(buf);
    }

    close(sockfd);
    return 0;
}
