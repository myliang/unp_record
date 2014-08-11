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

#include <limits.h> /* for OPEN_MAX */
#include <poll.h>

void str_echo(int sockfd);
void sig_chld(int signo);

int main(int argc, char **argv) {
  int i, n, maxfd, listenfd, connfd, sockfd;
  unsigned long maxi;
  int nready;
  char buf[2048];
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  struct pollfd client[1024];

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    fprintf(stderr, "socket create error");
    exit(0);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(8003);

  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr, "bind error");
    exit(0);
  }

  if (listen(listenfd, 0) < 0) {
    fprintf(stderr, "listen error");
    exit(0);
  }

  client[0].fd = listenfd;
  client[0].events = POLLRDNORM;

  for (i = 1; i < 1024; i++) {
    client[i].fd = -1;
  }

  maxi = 0;

  for (;;) {
    nready = poll(client, maxi + 1, 10);
    if (nready == 0) {
      fprintf(stderr, "poll timeout");
      exit(0);
    }
    if (nready < 0) {
      fprintf(stderr, "poll error");
      exit(0);
    }

    printf("nready= %d\n", nready);

    if (client[0].revents & POLLRDNORM) {
      clilen = sizeof(cliaddr);
      connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
      if (connfd < 0) {
        fprintf(stderr, "accept error");
        exit(0);
      }

      for (i = 1; i < 1024; i++) {
        if (client[i].fd < 0) {
          client[i].fd = connfd;
          break;
        }
      }

      if (i == 1024) {
        fprintf(stderr, "too many clients");
        exit(0);
      }

      client[i].events = POLLRDNORM;
      if (i > maxi)
        maxi = i;

      if (--nready <= 0)
        continue;
    }

    for (i = 0; i < maxi; i++) {
      if ((sockfd = client[i].fd) < 0) {
        continue;
      }

      if (client[i].revents & (POLLRDNORM|POLLERR)) {
        if ((n = read(sockfd, buf, 2048)) < 0) {
          if (errno == ECONNRESET) {
            // connection reset by client
            close(sockfd);
            client[i].fd = -1;
          } else {
            fprintf(stderr, "read error");
            exit(0);
          }
        } else if (n == 0) {
          // connection closed by client
          close(sockfd);
          client[i].fd = -1;
        } else {
          write(sockfd, buf, n);
        }

        if (--nready <= 0)
          break;
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
