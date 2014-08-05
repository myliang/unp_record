#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/time.h>

#include <errno.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

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
    servaddr.sin_port = htons(8002);

    if (connect(sockfd[i], (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
      fprintf(stderr, "connect error");
      exit(0);
    }
  }
  str_cli(stdin, sockfd[0]);

  exit(0);
}

void str_cli(FILE *fp, int sockfd) {
  int maxfdp1;
  fd_set rset;
  char sendline[2048], recvline[2048];

  FD_ZERO(&rset);
  for (;;) {
    FD_SET(fileno(fp), &rset);
    FD_SET(sockfd, &rset);

    maxfdp1 = max(fileno(fp), sockfd) + 1;
    select(maxfdp1, &rset, NULL, NULL, NULL);

    if (FD_ISSET(sockfd, &rset)) {
      // socket is readable
      if (readline(sockfd, recvline, 2048) == 0) {
        fprintf(stderr, "server terminated prematrurely");
        exit(-1);
      }

      fputs(recvline, stdout);
    }

    if (FD_ISSET(fileno(fp), &rset)) {
      // input is readable
      if (fgets(sendline, 2048, fp) == NULL) {
        return ;
      }

      write(sockfd, sendline, strlen(sendline));
    }
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

