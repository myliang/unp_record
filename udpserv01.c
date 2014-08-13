#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void dg_echo(int sockfd, struct sockaddr_in *addr, socklen_t addrlen);

int main (int argc, char const* argv[])
{
  int sockfd;
  struct sockaddr_in servaddr, cliaddr;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "socket error\n");
    exit(0);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(8020);

  if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr, "bind error");
    exit(0);
  }

  dg_echo(sockfd, &cliaddr, sizeof(cliaddr));
  return 0;
}

void dg_echo(int sockfd, struct sockaddr_in *addr, socklen_t addrlen) {
  int n;
  socklen_t len;
  char msg[2048];

  for(;;) {
    len = addrlen;
    n = recvfrom(sockfd, msg, 2048, 0, (struct sockaddr *)addr, &len);
    fprintf(stdout, "%s\n", inet_ntoa(addr->sin_addr));
    sendto(sockfd, msg, n, 0, (struct sockaddr *)addr, len);
  }
}
