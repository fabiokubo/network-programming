#include "utils.hpp"

// Array of registered users
vector<User> users;

// Prints to both terminal and log file a message regarding sent message
void log_message(string ipSender, int portNumberSender, char * ipReceiver, int portNumberReceiver, string message) {
    FILE *fp;
    fp = fopen ("log.txt", "a");
    printf("New message from %s:%d to %s:%d - Content: %s. Unix timestamp: %lu.\n",
        ipSender.c_str(), portNumberSender, ipReceiver, portNumberReceiver,
        message.c_str(), (unsigned long)time(NULL));
    fprintf(fp, "New message from %s:%d to %s:%d - Content: %s. Unix timestamp: %lu.\n",
        ipSender.c_str(), portNumberSender, ipReceiver, portNumberReceiver,
        message.c_str(), (unsigned long)time(NULL));
    fclose(fp);
}

// Prints to both terminal and log file a message regarding new user register
void log_enter(User *user) {
    FILE *fp;
    fp = fopen ("log.txt", "a");
    printf("New user %s with ip %s, port number %d and TCP port number %d registered. Unix timestamp: %lu.\n",
        user->nickname.c_str(), user->iPAddress.c_str(), user->portNumber,
        user->portNumberTCP, (unsigned long)time(NULL));
    fprintf(fp, "New user %s with ip %s, port number %d and TCP port number %d registered. Unix timestamp: %lu.\n",
        user->nickname.c_str(), user->iPAddress.c_str(), user->portNumber,
        user->portNumberTCP, (unsigned long)time(NULL));
    fclose(fp);
}

//
void log_exit(string nickname) {
    FILE *fp;
    fp = fopen ("log.txt", "a");
    printf("%s exited. Unix timestamp: %lu.\n", nickname.c_str(), (unsigned long)time(NULL));
    fprintf(fp, "%s exited. Unix timestamp: %lu.\n", nickname.c_str(), (unsigned long)time(NULL));
    fclose(fp);
}

// Validates number of paramenters
void validate_args(int argc, char **argv){
    if (argc != 2) {
        printf("Error: ./program <PortNumber>\n");
        exit(EXIT_FAILURE);
    }
}

// Populates the struct sockaddr_in with informations such as socket connection
//type,
void initialize_server_address(struct sockaddr_in * server_address, int portNumber){
    bzero(server_address, sizeof(* server_address));
    server_address->sin_family      = AF_INET;
    server_address->sin_addr.s_addr = htonl(INADDR_ANY); //Convert long int to big endian (if necessary)
    server_address->sin_port        = htons(portNumber); //Convert long int to big endian (if necessary)
}

// Assign to given socket a sockaddr struct containing address informations
void bind_name_to_socket(struct sockaddr * servaddr, int sockfd){
    int bind_result = bind(sockfd, servaddr, sizeof(* servaddr));
    if (bind_result < 0) {
        die("Error in method bind_name_to_socket.\n");
    }
}

// Sends a message to the provided user using given socket, message, sockaddr
//struct and its length
void send_message_to_user(int sockfd, const char * buf, int recv_len, struct sockaddr * peer_address, socklen_t slen){
    if (sendto(sockfd, buf, recv_len, 0, peer_address, slen) == -1){
        die("Error: sending message.\n"); // <-----
    }
}

// Creates a list with the registered users and send it to the solicitant user
void send_users_list(int sockfd, struct sockaddr * peer_address, socklen_t slen){
    int i, n;
    char message[BUFLEN], * aux;
    n = sprintf(message, "Connected Users:\n");
    aux = message + n;
    for (i = 0; i < users.size(); i++) {
        n = sprintf(aux, "%d - %s\n", i, users[i].nickname.c_str());
        aux += n;
    }
    send_message_to_user(sockfd, message, strlen(message), peer_address, slen);
}

// Creates a new user struct and populates it with received info
void add_new_user(char * buf, int recv_len, struct sockaddr_in * peer_address){
    int i;
    User newUser;
    i = get_user_index_by_nickname(users, get_nickname(buf));
    if (i>=0){
        users[i].portNumber = ntohs(peer_address->sin_port);
        users[i].iPAddress.assign(inet_ntoa(peer_address->sin_addr));
        users[i].portNumberTCP = atoi(get_message(buf).c_str());
        users[i].nickname.assign(get_nickname(buf));
        log_enter(&users.at(i));
    }else{
        newUser.portNumber = ntohs(peer_address->sin_port);
        newUser.iPAddress.assign(inet_ntoa(peer_address->sin_addr));
        newUser.portNumberTCP = atoi(get_message(buf).c_str());
        newUser.nickname.assign(get_nickname(buf));
        users.push_back(newUser);
        log_enter(&newUser);
    }
}

// Gets the destination user and sends to it given message
void handle_text_Message(int sockfd, struct sockaddr_in * sender_address, char * buf){
    struct sockaddr_in address;
    socklen_t slen=sizeof(address);
    char str[INET_ADDRSTRLEN], message[BUFLEN];
    int userIndex;

    userIndex = get_user_index_by_ip(users, get_nickname(buf));;
    if(userIndex < 0) {
        userIndex = get_user_index_by_nickname(users, get_nickname(buf));;
    }

    if(userIndex >= 0) {
        //
        string text = get_message(buf);
        message[0] = TEXT_MESSAGE;
        strcpy(&message[1], text.c_str());
        //
        inet_ntop(AF_INET, &(sender_address->sin_addr), str, INET_ADDRSTRLEN);
        log_message(users[userIndex].iPAddress, users[userIndex].portNumber, str, ntohs(sender_address->sin_port), message);
        initialize_address_by_user(&address, users[userIndex]);
        send_message_to_user(sockfd, message, sizeof(message), (struct sockaddr *) &address, slen);
    }
}

// Gets the destination user and sends to it given message
void provide_transfer_info(int sockfd, struct sockaddr_in * sender_address, char * buf){
    struct sockaddr_in address;
    socklen_t slen=sizeof(address);
    char str[INET_ADDRSTRLEN], message[BUFLEN];
    string message_content;
    int userIndex;

    userIndex = get_user_index_by_ip(users, get_nickname(buf));;
    if(userIndex < 0) {
        userIndex = get_user_index_by_nickname(users, get_nickname(buf));;
    }

    if(userIndex >= 0) {
        //
        string text = get_message(buf);
        message[0] = TRANSFER_MESSAGE;
        message_content = users[userIndex].iPAddress + " " + std::to_string(users[userIndex].portNumberTCP);
        strcpy(&message[1], message_content.c_str());
        //
        inet_ntop(AF_INET, &(sender_address->sin_addr), str, INET_ADDRSTRLEN);
        log_message(users[userIndex].iPAddress, users[userIndex].portNumber, str, ntohs(sender_address->sin_port), message);
        //initialize_address_by_user(&address, users[userIndex]);
        send_message_to_user(sockfd, message, sizeof(message), (struct sockaddr *) &address, slen);

    }
}

//
void handleExit(char * buf){
  int userIndex = get_user_index_by_nickname(users, get_nickname(buf));

  if(userIndex >= 0){
    log_exit(users[userIndex].nickname);
    users.erase(users.begin() + userIndex);
  }
}

// Switch through possible message types and process it
void process_message(int sockfd, char * buf, int recv_len, struct sockaddr * peer_address, socklen_t slen){
    switch(buf[0]){
        case REGISTER_USER:
            add_new_user(buf, recv_len, (struct sockaddr_in *) peer_address);
            send_users_list(sockfd, peer_address, slen);
            break;
        case TEXT_MESSAGE:
            handle_text_Message(sockfd, (struct sockaddr_in *) peer_address, buf);
            break;
        case LIST_MESSAGE:
            send_users_list(sockfd, peer_address, slen);
            break;
        case TRANSFER_MESSAGE:
            provide_transfer_info(sockfd, (struct sockaddr_in *) peer_address, buf);
            break;
        case EXIT_MESSAGE:
            handleExit(buf);
            break;
    }
}

// Wait for any client's message and proceess it
int receive_message_from_user(int sockfd, char * buf, struct sockaddr * peer_address, socklen_t * slen){
    int recv_len;
    if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, peer_address, slen)) == -1){
        printf("Error: receiving message.\n");
        exit(EXIT_FAILURE);
    }
    process_message(sockfd, buf, recv_len, peer_address, * slen);
    return recv_len;
}

int main(int argc, char **argv){
    struct sockaddr_in server_address, peer_address;
    socklen_t slen;
    int sockfd, recv_len;
    char buf[BUFLEN];

    printf("Starting server...\n");

    validate_args(argc, argv);
    sockfd = create_new_socket();
    initialize_server_address(&server_address, atoi(argv[1]));
    bind_name_to_socket((struct sockaddr *) &server_address, sockfd);

    //loops forever
    for(;;){
        printf("Waiting for data...\n");
        fflush(stdout);

        //try to receive some data, this is a blocking call
        slen = sizeof(peer_address);
        memset(buf,0,sizeof(buf)); // Buffer filled with zeroes
        recv_len = receive_message_from_user(sockfd, buf, (struct sockaddr *) &peer_address, &slen);

        //print details of the client/peer and the data received
        //printf("Received packet from %s:%d\n", inet_ntoa(peer_address.sin_addr), ntohs(peer_address.sin_port));
        //printf("Data: %s\n" , buf);
    }

    return 0;
}
