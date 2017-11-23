#include<vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  char iPAddress[20];
  char nickname[50];
} User;

enum MESSAGE_TYPE {
  REGISTER_USER=33,
  TEXT_MESSAGE=32
};

int getUserIndexByNickname(vector<User> users, char * nickname) {
  int i;

  for(i = 0; i < users.size(); i++) {
    if(strcmp(users[i].nickname, nickname) == 0) {
      return i;
    }
  }
  return -1;
}
