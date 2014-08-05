#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include <sys/select.h>
#include <sys/time.h>

void str_echo(int sockfd);
void sig_chld(int signo);

int main(int argc, char **argv) {
  int i, n, maxi, maxfd, listenfd, connfd, sockfd;
  int nready, client[FD_SETSIZE];
  fd_set rset, allset;
  char buf[2048];
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
  servaddr.sin_port = htons(8002);

  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr, "bind error");
    exit(0);
  }

  if (listen(listenfd, 0) < 0) {
    fprintf(stderr, "listen error");
    exit(0);
  }

  maxfd = listenfd; // initialize
  maxi = -1; // index into client[] array
  for(i = 0; i < FD_SETSIZE; i++)
    client[i] = -1; // -1 inidcates available entry

  FD_ZERO(&allset);
  FD_SET(listenfd, &allset);

  for (;;) {
    rset = allset;
    nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

    printf("nready= %d\n", nready);

    if (FD_ISSET(listenfd, &rset)) {
      // new client connection
      clilen = sizeof(cliaddr);
      connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

      for (i = 0; i < FD_SETSIZE; i++) {
        if (client[i] < 0) {
          client[i] = connfd; // save descriptor
          break;
        }

      }
      if (i == FD_SETSIZE) {
        fprintf(stderr, "too many clients");
        exit(0);
      }

      FD_SET(connfd, &allset); // add new descriptor to set
      if (connfd > maxfd)
        maxfd = connfd; // for select

      if (i > maxi)
        maxi = i; // max index in client[] array

      if (--nready <= 0)
        continue ; // no more readable descriptor
    }

    for (i = 0; i < maxi; i++) {
      // check all clients for data
      if ((sockfd = client[i]) < 0) {
        continue;
      }

      if (FD_ISSET(sockfd, &rset)) {
        if ((n = read(sockfd, buf, 2048)) == 0) {
          // connection closed by client
          close(sockfd);
          FD_CLR(sockfd, &allset);
          client[i] = -1;
        } else
          write(sockfd, buf, n);

        if (--nready <= 0)
          break; // no more readable descriptor
      }
    }
  }
}

void str_echo(int sockfd) {
  // fprintf(stderr, "enter str_echo");
  ssize_t n;
  char buf[2048];

again:
  while ((n = read(sockfd, buf, 2048)) > 0){
    // printf("read buff: %d, %s", n, buf);
    if (write(sockfd, buf, n) < 0) {
      fprintf(stderr, "write error\n");
      exit(0);
    }
    // printf("write finished %d\n", n);
  }
  if (n < 0 && errno == EINTR)
    goto again;
  else if (n < 0)
    fprintf(stderr, "str_echo: read error\n");
}

void sig_chld(int signo) {
  pid_t pid;
  int stat;
  while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
    printf("child %d terminated\n", pid);
  }
  // printf("sig_chld waitpid.pid = %d\n", pid);
}
