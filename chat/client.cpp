#include "utils.hpp"

string filePath;

// Validates number of paramenters
void validate_parameters(int argc, char **argv){
    if (argc != 3) {
        printf("Error: ./client <IPAddress> <PorNumber>\n");
        exit(EXIT_FAILURE);
    }
}

// Sends a message to the server using given socket, message, sockaddr struct
//and its length
void send_message_to_server(int sockfd, char * message, struct sockaddr * server_address, socklen_t slen){
    if (sendto(sockfd, message, strlen(message) , 0 , server_address, slen)==-1){
        die("Error: sendto() method.\n");
    }
}

// Receives message from server
void receive_message_from_server(int sockfd, char * buf, struct sockaddr * server_address, socklen_t * slen){
    if (recvfrom(sockfd, buf, BUFLEN, 0, server_address, slen) == -1){
        die("Error: recvfrom() method.\n");
    }
}

// Create user registration message to server
void send_register_message(int sockfd, char * nickname, char * portNumberTCP, struct sockaddr * server_address, socklen_t slen){
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
    send_message_to_server(sockfd, message, server_address, slen);
}

// Creates a message containing a text message and sends it
void send_text_message(int sockfd, char * message_content, struct sockaddr * server_address, socklen_t slen){
    char message[BUFLEN];
    //first message
    message[0] = TEXT_MESSAGE;
    //copying nickname into first message
    strcpy(&message[1], message_content);
    //send message
    send_message_to_server(sockfd, message, server_address, slen);
}

// Creates a message containing a text message and sends it
void send_list_message(int sockfd, struct sockaddr * server_address, socklen_t slen){
    char message[BUFLEN];
    //first message
    message[0] = LIST_MESSAGE;
    //send message
    send_message_to_server(sockfd, message, server_address, slen);
}

// Creates a message containing a file transfer request and sends it
void send_transfer_message(int sockfd, char * user, struct sockaddr * server_address, socklen_t slen){
    char message[BUFLEN];
    string strUser(user);
    string message_content;
    //first message
    message[0] = TRANSFER_MESSAGE;
    //setting nickname and portNumberTCP
    message_content = strUser + " ";
    //copying nickname into first message
    strcpy(&message[1], message_content.c_str());
    //send message
    send_message_to_server(sockfd, message, server_address, slen);
}

//
void send_exit_message(int sockfd, struct sockaddr * server_address, socklen_t slen, char * nickname){

    char message[BUFLEN];

    //first message
    message[0] = EXIT_MESSAGE;
    strcpy(&message[1], nickname);

    send_message_to_server(sockfd, message, server_address, slen);
}

// Print instructions regarding possible commands
void print_instructions(){
    printf("\nInstructions:\n");
    printf("- Command to send message: M <nickname> <message_content>\n");
    printf("- Command to send file: T <nickname> <file_name>\n");
    printf("- Command to list users: L\n");
    printf("- Command to change nickname: N\n");
    printf("- Command to exit: exit\n");
    printf("\n> ");
}

// Switches between possible commands and execute them
void handle_input(char buf[BUFLEN], int sockfd, struct sockaddr * server_address, socklen_t slen, char * nickname){
    // Handle message sending command
    if(buf[0] == 'M') {
        send_text_message(sockfd, &buf[2], server_address, slen);
        printf("\n> ");
    }
    // Handle file transfer command
    else if (buf[0] == 'T') {
        string user = get_nickname(&buf[1]);
        string path = get_message(&buf[1]);
        filePath = path;
        printf("ueva %s\n", filePath.c_str());
        send_transfer_message(sockfd, const_cast<char*>(user.c_str()), server_address, slen);
    }
    // Handle list users command
    else if(buf[0] == 'L') {
        send_list_message(sockfd, server_address, slen);
    }
    // Handle user updating command
    else if(buf[0] == 'N') {
        char nickname[50], portNumberTCP[10];
        printf("Enter your nickname: ");
        fgets(nickname , 50 , stdin);
        printf("Enter a port Number to TCP communication: ");
        fgets(portNumberTCP , 10 , stdin);
        send_register_message(sockfd, nickname, portNumberTCP, (struct sockaddr *) &server_address, slen);
    }
    // Handle exit command
    else if(strncmp(buf, "exit\n", BUFLEN) == 0){
        send_exit_message(sockfd, server_address, slen, nickname); // <-----
        close(sockfd);
        printf("Bye bye!\n");
        exit(EXIT_SUCCESS);
    }
    else {
        printf("Invalid command.. Please, try again\n");
        print_instructions();
    }
}

//
void handle_response_type(char *buf){
    // Handle message response
    if(buf[0] == TEXT_MESSAGE) {
        printf("\rFrom server: %s", &buf[1]);
        fflush(stdout);
        printf("\n> ");
        fflush(stdout);
    }
    // Handle file transfer response
    else if (buf[0] == TRANSFER_MESSAGE) {
        string iPAddress = get_ip_address(buf);
        string TCPport = get_tcp_port(buf);
        printf("Obtained infos for transfer: ip %s and port %s and path is %s\n",
            iPAddress.c_str(), TCPport.c_str(), filePath.c_str());
        fflush(stdout);
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

    sockfd = create_new_socket();
    initialize_address(argv[1], &server_address, atoi(argv[2]));

    printf("Welcome!\n");
    printf("Enter your nickname: ");
    fgets(nickname , 50 , stdin);
    printf("Enter a port Number to TCP communication: ");
    fgets(portNumberTCP , 10 , stdin);

    print_instructions();

    send_register_message(sockfd, nickname, portNumberTCP, (struct sockaddr *) &server_address, slen);

    //receive list of users
    memset(bufServer,'\0', BUFLEN); // Fills buffer with '\0' char
    receive_message_from_server(sockfd, bufServer, (struct sockaddr *) &server_address, &slen);
    printf("%s\n", &bufServer[1]);
    printf("Type your commands: \n> ");

    fflush(stdout);
    process_id = fork();

    if(process_id != 0) {//if is main fork
        //listens to user's commands
        for(;;){
            memset(bufUser,'\0', BUFLEN); // Fills buffer with '\0' char
            fgets(bufUser, BUFLEN, stdin);
            handle_input(bufUser, sockfd, (struct sockaddr *)&server_address, slen, nickname);
        }
    }
    else{ //if is child fork
        //listens to server
        for(;;){
            memset(bufServer,'\0', BUFLEN); // Fills buffer with '\0' char
            receive_message_from_server(sockfd, bufServer, (struct sockaddr *) &server_address, &slen);
            handle_response_type(bufServer);
        }
    }


    return 0;
}
