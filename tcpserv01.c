#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

void str_echo(int sockfd);

int main(int argc, char **argv) {
  int listenfd, connfd;
  pid_t childpid;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    fprintf(stderr, "socket create error");
    exit(0);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(8001);

  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr, "bind error");
    exit(0);
  }

  if (listen(listenfd, 0) < 0) {
    fprintf(stderr, "listend error");
    exit(0);
  }

  for ( ; ; ) {
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

    if ((childpid = fork()) == 0) {
      close(listenfd);
      str_echo(connfd);
      exit(0);
    }

    close(connfd);
  }
}

void str_echo(int sockfd) {
  ssize_t n;
  char buf[2048];

again:
  while (n = read(sockfd, buf, 2048) > 0)
    write(sockfd, buf, n);

  if (n < 0 && errno == EINTR)
    goto again;
  else if (n < 0)
    fprintf(stderr, "str_echo: read error");
}
