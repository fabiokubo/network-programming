#include<vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFLEN 512  //Max length of buffer

using namespace std;

typedef struct User {
  int portNumber;
  int portNumberTCP;
  string iPAddress;
  string nickname;
} User;

// Message codes
enum MESSAGE_TYPE {
    REGISTER_USER=33,
    TEXT_MESSAGE=34,
    EXIT_MESSAGE=35,
    LIST_MESSAGE=36,
    TRANSFER_MESSAGE=37
};

// Exit program throwing an error message
void die(string s){
    perror(s.c_str());
    exit(1);
}

// Returns the index of user with given nickname
int get_user_index_by_nickname(vector<User> users, string nickname) {
    int i;
    for(i = 0; i < users.size(); i++) {
        if(users[i].nickname == nickname) {
            return i;
        }
    }
    return -1;
}

// Returns the index of user with given IP
int get_user_index_by_ip(vector<User> users, string ip) {
    int i;
    for(i = 0; i < users.size(); i++) {
        if(users[i].iPAddress == ip) {
            return i;
        }
    }
    return -1;
}

// Finds the nickname by parsing the buffer
string get_nickname(char * buf){
    size_t spacePosition;
    string aux(buf);
    spacePosition = aux.find(" ");
    return aux.substr(1, spacePosition - 1);
}
// Finds the ip address by parsing the buffer
string get_ip_address(char * buf){
    return get_nickname(buf);
}

// Finds the message by parsing the buffer
string get_message(char * buf){
    size_t spacePosition;
    string aux(buf);
    spacePosition = aux.find(" ");
    return aux.substr(spacePosition + 1);
}
// Finds the message by parsing the buffer
string get_tcp_port(char * buf){
    return get_message(buf);
}

// Populates the struct sockaddr_in with informations such as server IP
//(converted to binary), port and socket connection type
void initialize_address(const char * server_ip, struct sockaddr_in * address, int portNumber){
    bzero(address, sizeof(* address));
    address->sin_family      = AF_INET;
    address->sin_port        = htons(portNumber);
    //Converts server address IP from string to binary and saves in address
    if (inet_pton(AF_INET, server_ip, &(address->sin_addr) ) <= 0) {
        printf("Error to convert server ip from text to binary.\n");
        exit(EXIT_FAILURE);
    }
}

// Populates the struct sockaddr_in with user informations
void initialize_address_by_user(struct sockaddr_in * address, User user){
    initialize_address(user.iPAddress.c_str(), address, user.portNumber);
}

//Creates a new socket and returns its descriptor
int create_new_socket(){
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
       die("Failed to create new socket.");
    }
    return sockfd;
}
