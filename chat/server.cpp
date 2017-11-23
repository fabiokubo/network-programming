#include "utils.hpp"

//Connected users
vector<User> users;

void log_message(string ipSender, int portNumberSender, char * ipReceiver, int portNumberReceiver, string message) {
  FILE *fp;
  fp = fopen ("log.txt", "a");
  printf("Nova mensagem de %s:%d para %s:%d - Conteudo: %s. Unix timestamp: %lu.\n", ipSender.c_str(), portNumberSender,
    ipReceiver, portNumberReceiver, message.c_str(), (unsigned long)time(NULL));

  fprintf(fp, "Nova mensagem de %s:%d para %s:%d - Conteudo: %s. Unix timestamp: %lu.\n", ipSender.c_str(), portNumberSender,
    ipReceiver, portNumberReceiver, message.c_str(), (unsigned long)time(NULL));
  fclose(fp);
}

void log_enter(string nickname, string ipAddress, int portNumber) {
  FILE *fp;
  fp = fopen ("log.txt", "a");
  printf("Novo usario %s de ip %s e porta numero %d entrou na sala. Unix timestamp: %lu.\n", nickname.c_str(),
   ipAddress.c_str(), portNumber, (unsigned long)time(NULL));

  fprintf(fp, "Novo usario %s de ip %s e porta numero %d entrou na sala. Unix timestamp: %lu.\n", nickname.c_str(),
   ipAddress.c_str(), portNumber, (unsigned long)time(NULL));
  fclose(fp);
}

void log_exit(string nickname) {
  FILE *fp;
  fp = fopen ("log.txt", "a");
  printf("%s saiu da sala. Unix timestamp: %lu.\n", nickname.c_str(), (unsigned long)time(NULL));
  fprintf(fp, "%s saiu da sala. Unix timestamp: %lu.\n", nickname.c_str(), (unsigned long)time(NULL));
  fclose(fp);
}

void validate_args(int argc, char **argv){
    if (argc != 2) {
        printf("Error: ./program <PortNumber>\n");
        exit(EXIT_FAILURE);
    }
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

void sendMessageToUser(int sockfd, const char * buf, int recv_len, struct sockaddr * peer_address, socklen_t slen){
  if (sendto(sockfd, buf, recv_len, 0, peer_address, slen) == -1){
    printf("Error: sending message.\n");
    exit(EXIT_FAILURE);
  }
}

void sendUsersList(int sockfd, struct sockaddr * peer_address, socklen_t slen){
  int i, n;
  char message[BUFLEN], * aux;

  n = sprintf(message, "Connected Users:\n");
  aux = message + n;
  for (i = 0; i < users.size(); i++) {
    // id - nickname
    n = sprintf(aux, "%d - %s\n", i, users[i].nickname.c_str());
    aux += n;
  }
  sendMessageToUser(sockfd, message, strlen(message), peer_address, slen);
}

string getNickname(char * buf){
  size_t spacePosition;
  string aux(buf);
  spacePosition = aux.find(" ");
  return aux.substr(1, spacePosition - 1);
}

string getMessage(char * buf){
  size_t spacePosition;
  string aux(buf);
  spacePosition = aux.find(" ");
  return aux.substr(spacePosition + 1);
}

void addNewUser(char * buf, int recv_len, struct sockaddr_in * peer_address){
  User newUser;

  newUser.portNumber = ntohs(peer_address->sin_port);
  newUser.iPAddress.assign(inet_ntoa(peer_address->sin_addr));
  newUser.portNumberTCP = atoi(getMessage(buf).c_str());
  newUser.nickname.assign(getNickname(buf));
  users.push_back(newUser);
  log_enter(newUser.nickname, newUser.iPAddress, newUser.portNumber);
}

void handleTextMessage(int sockfd, struct sockaddr_in * sender_address, char * buf){
  int userIndex;
  string message;
  struct sockaddr_in address;
  socklen_t slen=sizeof(address);
  char str[INET_ADDRSTRLEN];

  userIndex = getUserIndexByNickname(users, getNickname(buf));
  message = getMessage(buf);

  if(userIndex >= 0) {

    inet_ntop(AF_INET, &(sender_address->sin_addr), str, INET_ADDRSTRLEN);
    log_message(users[userIndex].iPAddress, users[userIndex].portNumber, str, ntohs(sender_address->sin_port), message);

    initializeAddressByUser(&address, users[userIndex]);
    sendMessageToUser(sockfd, message.c_str(), message.length(), (struct sockaddr *) &address, slen);
  }
}

void handleExit(char * buf){
  int userIndex = getUserIndexByNickname(users, getNickname(buf));

  if(userIndex >= 0){
    log_exit(users[userIndex].nickname);
    users.erase(users.begin() + userIndex);
  }
}

void processMessage(int sockfd, char * buf, int recv_len, struct sockaddr * peer_address, socklen_t slen){

  switch(buf[0]){
    case REGISTER_USER:
      addNewUser(buf, recv_len, (struct sockaddr_in *) peer_address);
      sendUsersList(sockfd, peer_address, slen);
      break;
    case TEXT_MESSAGE:
      handleTextMessage(sockfd, (struct sockaddr_in *) peer_address, buf);
      break;
    case EXIT_MESSAGE:
      handleExit(buf);
      break;

  }
}

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
      memset(buf,0,sizeof(buf));
      recv_len = receiveMessageFromUser(sockfd, buf, (struct sockaddr *) &peer_address, &slen);

      //print details of the client/peer and the data received
      //printf("Received packet from %s:%d\n", inet_ntoa(peer_address.sin_addr), ntohs(peer_address.sin_port));
      //printf("Data: %s\n" , buf);
  }

  return 0;
}
