#include "utils.h"

#define MAX_CLIENTS 10

/*Global variables*/
//array with all known clients
Client clients[MAX_CLIENTS];

//quantity of known clients
int n_clients = 0;


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

void addNewUser(char * buf, int recv_len, struct sockaddr_in * peer_address){

  clients[n_clients].portNumber = ntohs(peer_address->sin_port);
  strcpy(clients[n_clients].iPAddress, inet_ntoa(peer_address->sin_addr));

  buf[recv_len-1] = '\0';
  strcpy(clients[n_clients].nickname, &buf[1]);

  printf("Received packet from %s:%d\n", inet_ntoa(peer_address->sin_addr), ntohs(peer_address->sin_port));
  printf("New user added: %s, Address - %s : %d.\n", clients[n_clients].nickname, clients[n_clients].iPAddress, clients[n_clients].portNumber);

  n_clients++;
}

void processMessage(char * buf, int recv_len, struct sockaddr_in * peer_address){

    switch(buf[0]){
      case REGISTER_USER :
        printf("Teste\n");
        addNewUser(buf, recv_len, peer_address);
        break;
    }
}

int receiveMessageFromClient(int sockfd, char * buf, struct sockaddr * peer_address, socklen_t * slen){
  int recv_len;

  if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, peer_address, slen)) == -1){
    printf("Error: receiving message.\n");
    exit(EXIT_FAILURE);
  }

  processMessage(buf, recv_len, (struct sockaddr_in*) peer_address);

  return recv_len;
}

void sendMessageToClient(int sockfd, char * buf, int recv_len, struct sockaddr * peer_address, socklen_t slen){
  if (sendto(sockfd, buf, recv_len, 0, peer_address, slen) == -1){
    printf("Error: sending message.\n");
    exit(EXIT_FAILURE);
  }
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
      memset(buf,0,sizeof(buf));
      recv_len = receiveMessageFromClient(sockfd, buf, (struct sockaddr *) &peer_address, &slen);

      //print details of the client/peer and the data received
      printf("Received packet from %s:%d\n", inet_ntoa(peer_address.sin_addr), ntohs(peer_address.sin_port));
      printf("Data: %s\n" , buf);

      //now reply the client with the same data
      sendMessageToClient(sockfd, buf, recv_len, (struct sockaddr *) &peer_address, slen);
  }

  return 0;
}
