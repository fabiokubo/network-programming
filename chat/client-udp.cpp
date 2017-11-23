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

  printf("%s\n", buf);
}

void sendRequestMessage(int sockfd, char * nickname, struct sockaddr * server_address, socklen_t slen){

  char message[BUFLEN];

  //first message
  message[0] = REGISTER_USER;

  //copying nickname into first message
  strcpy(&message[1], nickname);

  sentMessageToServer(sockfd, message, server_address, slen);
}

void sendTextMessage(int sockfd, char * message_content, struct sockaddr * server_address, socklen_t slen){

  char message[BUFLEN];

  //first message
  message[0] = TEXT_MESSAGE;

  //copying nickname into first message
  strcpy(&message[1], message_content);

  sentMessageToServer(sockfd, message, server_address, slen);
}

void printInstructions(){
  printf("Instructions:\n");
  printf("- Command to send message: M <nickname> <message_content>\n");
  printf("- Command to send file: T <nickname> <file_name>\n");
  printf("- Command to exit: exit\n\n");
}

void handleInput(char buf[BUFLEN], int sockfd, struct sockaddr * server_address, socklen_t slen){

  //handle exit command
  if(strncmp(buf, "exit\n", BUFLEN) == 0){
    close(sockfd);
    printf("Bye bye!\n");
    exit(EXIT_SUCCESS);
  }

  if(buf[0] == 'M') {
    sendTextMessage(sockfd, &buf[2], server_address, slen);
  }
  else if (buf[0] == 'T') {
    printf("Transfer\n");
  }
  else {
    printf("Invalid command.. Please, try again\n");
    printInstructions();
  }
}

int main(int argc, char **argv)
{
    struct sockaddr_in server_address;
    socklen_t slen=sizeof(server_address);
    int sockfd;
    char buf[BUFLEN], nickname[50];

    validate_parameters(argc, argv);

    sockfd = create_new_socket();
    initializeAddress(argv[1], &server_address, atoi(argv[2]));

    printf("Welcome!\n");
    printInstructions();

    printf("Enter your nickname: ");
    fgets(nickname , 50 , stdin);

    sendRequestMessage(sockfd, nickname, (struct sockaddr *) &server_address, slen);

    for(;;){
        memset(buf,'\0', BUFLEN);
        printf("From server: \n");
        receiveMessageFromServer(sockfd, buf, (struct sockaddr *) &server_address, &slen);

        memset(buf,'\0', BUFLEN);
        printf("Digite algo: \n");
        fgets(buf, BUFLEN, stdin);
        handleInput(buf, sockfd, (struct sockaddr *)&server_address, slen);
    }
    return 0;
}
