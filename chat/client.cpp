#include "utils.hpp"

// Validates number of paramenters
void validate_parameters(int argc, char **argv){
    if (argc != 3) {
        printf("Error: ./client <IPAddress> <PorNumber>\n");
        exit(EXIT_FAILURE);
    }
}

// Sends a message to the server using given socket, message, sockaddr struct
//and its length
void sendMessageToServer(int sockfd, char * message, struct sockaddr * server_address, socklen_t slen){
    if (sendto(sockfd, message, strlen(message) , 0 , server_address, slen)==-1){
        die("Error: sendto() method.\n");
    }
}

// Receives message from server
void receiveMessageFromServer(int sockfd, char * buf, struct sockaddr * server_address, socklen_t * slen){
    if (recvfrom(sockfd, buf, BUFLEN, 0, server_address, slen) == -1){
        die("Error: recvfrom() method.\n");
    }
}

// Create user registration message to server
void sendRegisterMessage(int sockfd, char * nickname, char * portNumberTCP, struct sockaddr * server_address, socklen_t slen){
    char message[BUFLEN];
    nickname[strlen(nickname)-1] = '\0';
    portNumberTCP[strlen(portNumberTCP)-1] = '\0';
    string strNickname(nickname);
    string strPortNumberTCP(portNumberTCP);
    string message_content;
    //first message
    message[0] = REGISTER_USER;
    //setting nickname and portNumberTCP
    message_content = strNickname + " " + strPortNumberTCP;
    strcpy(&message[1], message_content.c_str());
    //send message
    sendMessageToServer(sockfd, message, server_address, slen);
}

// Creates a message containing a text message and sends it
void sendTextMessage(int sockfd, char * message_content, struct sockaddr * server_address, socklen_t slen){
    char message[BUFLEN];
    //first message
    message[0] = TEXT_MESSAGE;
    //remove \n
    //message_content[strlen(message_content) - 1] = '\0';
    //copying nickname into first message
    strcpy(&message[1], message_content);
    //send message
    sendMessageToServer(sockfd, message, server_address, slen);
}

// Creates a message containing a text message and sends it
void sendListMessage(int sockfd, struct sockaddr * server_address, socklen_t slen){
    char message[BUFLEN];
    //first message
    message[0] = LIST_MESSAGE;
    //send message
    sendMessageToServer(sockfd, message, server_address, slen);
}

//
void sendExitMessage(int sockfd, struct sockaddr * server_address, socklen_t slen, char * nickname){

    char message[BUFLEN];

    //first message
    message[0] = EXIT_MESSAGE;
    strcpy(&message[1], nickname);

    sendMessageToServer(sockfd, message, server_address, slen);
}

// Print instructions regarding possible commands
void printInstructions(){
    printf("\nInstructions:\n");
    printf("- Command to send message: M <nickname> <message_content>\n");
    printf("- Command to send file: T <nickname> <file_name>\n");
    printf("- Command to list users: L\n");
    printf("- Command to change nickname: N <nickname>\n");
    printf("- Command to exit: exit\n\n");
}

// Switches between possible commands and execute them
void handleInput(char buf[BUFLEN], int sockfd, struct sockaddr * server_address, socklen_t slen, char * nickname){
    // Handle message sending command
    if(buf[0] == 'M') {
        sendTextMessage(sockfd, &buf[2], server_address, slen);
    }
    // Handle file transfer command
    else if (buf[0] == 'T') {
        printf("Transfer\n"); //<-----
    }
    // Handle list users command
    else if(buf[0] == 'L') {
        sendListMessage(sockfd, server_address, slen);
    }
    // Handle user updating command
    else if(buf[0] == 'N') {
        //sendTextMessage(sockfd, &buf[2], server_address, slen);
    }
    // Handle exit command
    else if(strncmp(buf, "exit\n", BUFLEN) == 0){
        sendExitMessage(sockfd, server_address, slen, nickname); // <-----
        close(sockfd);
        printf("Bye bye!\n");
        exit(EXIT_SUCCESS);
    }
    else {
        printf("Invalid command.. Please, try again\n");
        printInstructions();
    }
}

int main(int argc, char **argv){
    pid_t process_id;
    struct sockaddr_in server_address;
    socklen_t slen=sizeof(server_address);
    int sockfd;
    char bufServer[BUFLEN], bufUser[BUFLEN], nickname[50], portNumberTCP[10];
    vector<User> users;

    validate_parameters(argc, argv);

    sockfd = createNewSocket();
    initializeAddress(argv[1], &server_address, atoi(argv[2]));

    printf("Welcome!\n");
    printf("Enter your nickname: ");
    fgets(nickname , 50 , stdin);

    printf("Enter a port Number to TCP communication: ");
    fgets(portNumberTCP , 10 , stdin);

    printInstructions();

    sendRegisterMessage(sockfd, nickname, portNumberTCP, (struct sockaddr *) &server_address, slen);

    //receive list of users
    memset(bufServer,'\0', BUFLEN); // Fills buffer with '\0' char
    receiveMessageFromServer(sockfd, bufServer, (struct sockaddr *) &server_address, &slen);
    printf("%s\n", bufServer);
    printf("Type your commands: \n> ");

    fflush(stdout);
    process_id = fork();

    if(process_id != 0) {//if is main fork
        //listens to user's commands
        for(;;){
            memset(bufUser,'\0', BUFLEN); // Fills buffer with '\0' char
            fgets(bufUser, BUFLEN, stdin);
            handleInput(bufUser, sockfd, (struct sockaddr *)&server_address, slen, nickname);
            printf("\n> ");
        }
    }
    else{ //if is child fork
        //listens to server
        for(;;){
            memset(bufServer,'\0', BUFLEN); // Fills buffer with '\0' char
            receiveMessageFromServer(sockfd, bufServer, (struct sockaddr *) &server_address, &slen);
            printf("\rFrom server: %s\n", bufServer);
            fflush(stdout);
            printf("> ", bufServer);
            fflush(stdout);
        }
    }


    return 0;
}
