#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sum.h"

#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <errno.h>

void str_cli(FILE *fp, int sockfd);
size_t readline(int fd, void *ptr, size_t maxlen);

int main(int argc, char** argv) {
  int i, sockfd[5];
  struct sockaddr_in servaddr;

  if (argc != 2) {
    fprintf(stderr, "usage: tcpcli <IPaddress>");
    exit(0);
  }

  for (i = 0; i < 5; i++) {
    sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      fprintf(stderr, "socket error");
      exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(8001);

    if (connect(sockfd[i], (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
      fprintf(stderr, "connect error");
      exit(0);
    }
  }
  str_cli(stdin, sockfd[0]);

  exit(0);
}

void str_cli(FILE *fp, int sockfd) {
  char sendline[2048];
  struct args args;
  struct result result;
  while (fgets(sendline, 2048, fp) != NULL) {
    if (sscanf(sendline, "%ld%ld", &args.arg1, &args.arg2) != 2) {
      printf("invalid input: %s", sendline);
      continue ;
    }

    write(sockfd, &args, sizeof(args));

    int n;
    if ((n = read(sockfd, &result, sizeof(result))) <= 0) {
      fprintf(stderr, "read error");
      exit(0);
    }

    printf("%ld\n", result.sum);
  }
}

size_t readline(int fd, void *ptr, size_t maxlen) {
  int n, rc;
  char c, *tptr;

  tptr = ptr;

  for (n = 1; n < maxlen; n++) {
again:
    rc = read(fd, &c, 1);
    // printf("rc = %d, char = %c\n", rc, c);
    if (rc == 1) {
      // printf("%c\n", c);
      *tptr++ = c;
      if (c == '\n')
        break;
    } else if (rc == 0) {
      *tptr = 0;
      return n - 1;
    } else {
      if (errno == EINTR)
        goto again;
      return -1;
    }
  }

  *tptr = 0;
  return n;
}

