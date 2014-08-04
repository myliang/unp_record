#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <errno.h>

void str_cli(FILE *fp, int sockfd);
size_t readline(int fd, void *ptr, size_t maxlen);

int main(int argc, char** argv) {
  int sockfd;
  struct sockaddr_in servaddr;

  if (argc != 2) {
    fprintf(stderr, "usage: tcpcli <IPaddress>");
    exit(0);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "socket error");
    exit(0);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);
  servaddr.sin_port = htons(8001);

  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr, "connect error");
    exit(0);
  }

  str_cli(stdin, sockfd);

  exit(0);
}

void str_cli(FILE *fp, int sockfd) {
  char sendline[2048], recvline[2048];
  while (fgets(sendline, 2048, fp) != NULL) {
    // fprintf(stderr, "sendline: %s", sendline);
    write(sockfd, sendline, strlen(sendline));
    // fprintf(stderr, "write finished\n");

    int n;
    if ((n = readline(sockfd, recvline, 2048)) < 0) {
      fprintf(stderr, "read error");
      exit(0);
    }

    // fprintf(stderr, "readline:%d:%s", n, recvline);

    fputs(recvline, stdout);
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
