#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>


#define LISTENQ 10
#define MAXDATASIZE 100

int main (int argc, char **argv) {
   int    listenfd, connfd, peerinfo;
   struct sockaddr_in servaddr;
   char   buf[MAXDATASIZE];
   time_t ticks;
   struct sockaddr_in peer;
   int peer_len;

   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("socket");
      exit(1);
   }

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family      = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port        = htons(6000);

   if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      perror("bind");
      exit(1);
   }
   
   if (listen(listenfd, LISTENQ) == -1) {
      perror("listen");
      exit(1);
   }


   for ( ; ; ) {
      if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
         perror("accept");
         exit(1);
      }

      ticks = time(NULL);
      snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
      write(connfd, buf, strlen(buf));

      peer_len = sizeof(peer);
      if (getpeername(connfd, (struct sockaddr *)&peer, &peer_len) == -1) {
          perror("getpeername() failed");
          return -1;
      }
      printf("Peer's IP address is: %s\n", inet_ntoa(peer.sin_addr));
      printf("Peer's port is: %d\n", (int) ntohs(peer.sin_port));

      close(connfd);
   }

   return(0);
}
