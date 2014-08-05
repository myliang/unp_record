#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

void str_echo(int sockfd);
void sig_chld(int signo);

int main(int argc, char **argv) {
  int listenfd, connfd;
  pid_t childpid;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    fprintf(stderr, "socket create error\n");
    exit(0);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(8001);

  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr, "bind error\n");
    exit(0);
  }

  if (listen(listenfd, 0) < 0) {
    fprintf(stderr, "listend error\n");
    exit(0);
  }

  signal(SIGCHLD, sig_chld);
  signal(SIGCHLD, sig_chld);

  for ( ; ; ) {
    clilen = sizeof(cliaddr);
    if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
      if (errno == EINTR)
        continue ;
      else {
        fprintf(stderr, "accept error");
        exit(0);
      }
    }

    if ((childpid = fork()) == 0) {
      close(listenfd);
      str_echo(connfd);
      exit(0);
    }

    close(connfd);
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
