#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int listenfd, connfd;
  socklen_t len;
  struct sockaddr_in servaddr, cliaddr;
  char buff[1024];
  time_t ticks;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(4333);

  bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  listen(listenfd, 120);

  for(;;) {
    printf("start...\n");
    len = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
    printf("connection from %s, prot %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    ticks = time(NULL);
    snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
    write(connfd, buff, strlen(buff));
    close(connfd);
  }
}
