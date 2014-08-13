#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void dg_cli(FILE *file, int sockfd, struct sockaddr_in *addr, socklen_t len);

int main (int argc, char const* argv[])
{
  int sockfd;
  struct sockaddr_in servaddr, cliaddr;

  if (argc != 2) {
    fprintf(stderr, "usage: updcli <IPaddress>\n");
    exit(0);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);
  servaddr.sin_port = htons(8020);

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  dg_cli(stdin, sockfd, &servaddr, sizeof(servaddr));
  return 0;
}

void dg_cli(FILE *fp, int sockfd, struct sockaddr_in *addr, socklen_t len){
  int n;
  char sendline[2084], recvline[2085];

  while (fgets(sendline, 2048, fp) != NULL) {
    sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)addr, len);
    n = recvfrom(sockfd, recvline, 2048, 0, NULL, NULL);
    recvline[n] = 0;
    fputs(recvline, stdout);
  }
}
