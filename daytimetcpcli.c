#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <errno.h>

int main(int argc, char **argv) {
  // client day time
  int connfd;
  struct sockaddr_in servaddr;
  char buff[1024];

  printf("host:%s\n", argv[1]);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);
  servaddr.sin_port = htons(4333);

  connfd = socket(AF_INET, SOCK_STREAM, 0);
  if (connfd < 0) {
    fprintf(stderr, "create socket error, exit\n");
    exit(0);
  }
  if (connect(connfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr, "can not connect to %s, exit\n", argv[1]);
    fprintf(stderr, "%s\n", strerror(errno));
    exit(0);
  }

  printf("buff_size =%d\n", sizeof(buff));
  read(connfd, buff, sizeof(buff));
  printf("%s\n", buff);
  close(connfd);
}
