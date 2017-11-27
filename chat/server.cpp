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
void log_enter(string nickname, string ipAddress, int portNumber) {
    FILE *fp;
    fp = fopen ("log.txt", "a");
    printf("New user %s with ip %s and port number %d registered. Unix timestamp: %lu.\n",
        nickname.c_str(), ipAddress.c_str(), portNumber, (unsigned long)time(NULL));
    fprintf(fp, "New user %s with ip %s and port number %d registered. Unix timestamp: %lu.\n",
        nickname.c_str(), ipAddress.c_str(), portNumber, (unsigned long)time(NULL));
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
void sendMessageToUser(int sockfd, const char * buf, int recv_len, struct sockaddr * peer_address, socklen_t slen){
    if (sendto(sockfd, buf, recv_len, 0, peer_address, slen) == -1){
        die("Error: sending message.\n"); // <-----
    }
}

// Creates a list with the registered users and send it to the solicitant user
void sendUsersList(int sockfd, struct sockaddr * peer_address, socklen_t slen){
    int i, n;
    char message[BUFLEN], * aux;
    n = sprintf(message, "Connected Users:\n");
    aux = message + n;
    for (i = 0; i < users.size(); i++) {
        n = sprintf(aux, "%d - %s\n", i, users[i].nickname.c_str());
        aux += n;
    }
    sendMessageToUser(sockfd, message, strlen(message), peer_address, slen);
}

// Finds the nickname by parsing the buffer
string getNickname(char * buf){
    size_t spacePosition;
    string aux(buf);
    spacePosition = aux.find(" ");
    return aux.substr(1, spacePosition - 1);
}

// Finds the message by parsing the buffer
string getMessage(char * buf){
    size_t spacePosition;
    string aux(buf);
    spacePosition = aux.find(" ");
    return aux.substr(spacePosition + 1);
}

// Creates a new user struct and populates it with received info
void addNewUser(char * buf, int recv_len, struct sockaddr_in * peer_address){
    int i;
    User newUser;
    i = getUserIndexByNickname(users, getNickname(buf));
    if (i>=0){
        users[i].portNumber = ntohs(peer_address->sin_port);
        users[i].iPAddress.assign(inet_ntoa(peer_address->sin_addr));
        users[i].portNumberTCP = atoi(getMessage(buf).c_str());
        users[i].nickname.assign(getNickname(buf));
        log_enter(users[i].nickname, users[i].iPAddress, users[i].portNumber);
    }else{
        newUser.portNumber = ntohs(peer_address->sin_port);
        newUser.iPAddress.assign(inet_ntoa(peer_address->sin_addr));
        newUser.portNumberTCP = atoi(getMessage(buf).c_str());
        newUser.nickname.assign(getNickname(buf));
        users.push_back(newUser);
        log_enter(newUser.nickname, newUser.iPAddress, newUser.portNumber);
    }
}

// Gets the destination user and sends to it given message
void handleTextMessage(int sockfd, struct sockaddr_in * sender_address, char * buf){
    struct sockaddr_in address;
    socklen_t slen=sizeof(address);
    char str[INET_ADDRSTRLEN];
    string message = getMessage(buf);
    int userIndex;

    userIndex = getUserIndexByNickname(users, getNickname(buf));;
    if(userIndex < 0) {
        userIndex = getUserIndexByIP(users, getNickname(buf));;
    }

    if(userIndex >= 0) {
        inet_ntop(AF_INET, &(sender_address->sin_addr), str, INET_ADDRSTRLEN);
        log_message(users[userIndex].iPAddress, users[userIndex].portNumber, str, ntohs(sender_address->sin_port), message);

        initializeAddressByUser(&address, users[userIndex]);
        sendMessageToUser(sockfd, message.c_str(), message.length(), (struct sockaddr *) &address, slen);
    }
}

//
void handleExit(char * buf){
  int userIndex = getUserIndexByNickname(users, getNickname(buf));

  if(userIndex >= 0){
    log_exit(users[userIndex].nickname);
    users.erase(users.begin() + userIndex);
  }
}

// Switch through possible message types and process it
void processMessage(int sockfd, char * buf, int recv_len, struct sockaddr * peer_address, socklen_t slen){
    switch(buf[0]){
        case REGISTER_USER:
            addNewUser(buf, recv_len, (struct sockaddr_in *) peer_address);
            sendUsersList(sockfd, peer_address, slen);
            break;
        case TEXT_MESSAGE:
            handleTextMessage(sockfd, (struct sockaddr_in *) peer_address, buf);
            break;
        case LIST_MESSAGE:
            sendUsersList(sockfd, peer_address, slen);
            break;
        case EXIT_MESSAGE:
            handleExit(buf);
            break;
    }
}

// Wait for any client's message and proceess it
int receiveMessageFromUser(int sockfd, char * buf, struct sockaddr * peer_address, socklen_t * slen){
    int recv_len;
    if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, peer_address, slen)) == -1){
        printf("Error: receiving message.\n");
        exit(EXIT_FAILURE);
    }
    processMessage(sockfd, buf, recv_len, peer_address, * slen);
    return recv_len;
}

int main(int argc, char **argv){
    struct sockaddr_in server_address, peer_address;
    socklen_t slen;
    int sockfd, recv_len;
    char buf[BUFLEN];

    printf("Starting server...\n");

    validate_args(argc, argv);
    sockfd = createNewSocket();
    initialize_server_address(&server_address, atoi(argv[1]));
    bind_name_to_socket((struct sockaddr *) &server_address, sockfd);

    //loops forever
    for(;;){
        printf("Waiting for data...\n");
        fflush(stdout);

        //try to receive some data, this is a blocking call
        slen = sizeof(peer_address);
        memset(buf,0,sizeof(buf)); // Buffer filled with zeroes
        recv_len = receiveMessageFromUser(sockfd, buf, (struct sockaddr *) &peer_address, &slen);

        //print details of the client/peer and the data received
        //printf("Received packet from %s:%d\n", inet_ntoa(peer_address.sin_addr), ntohs(peer_address.sin_port));
        //printf("Data: %s\n" , buf);
    }

    return 0;
}
