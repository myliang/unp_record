#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <error.h>

#define MAXFD 64

// extern int daemon_proc; // defined in error.c

int daemon_init(const char *pname, int facility) {
  int i;
  pid_t pid;

  if ((pid = fork()) < 0) {
    fprintf(stderr, "fork error\n");
    return -1;
  } else if (pid)
    _exit(0); // parent terminates

  // child 1 continues

  if (setsid() < 0) {
    fprintf(stderr, "setsid error\n");
    return -1;
  }

  signal(SIGHUP, SIG_IGN);
  if ((pid = fork()) < 0) {
    return -1;
  } else if (pid)
    _exit(0);  // child 1 terminateds

  // child 2 continues
  // daemon_proc = 1; // for err_xxx() functions
  chdir("/"); // change working directory

  // close off file descriptors
  for (i = 0; i < MAXFD; i++) {
    close(i);
  }

  // redirect stdin, stdout,)and stderr to /dev/null
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_WRONLY);
  open("/dev/null", O_RDWR);

  openlog(pname, LOG_PID, facility);

  return 0;
}

int main (int argc, char const* argv[])
{
  daemon_init(argv[0], 0);
  return 0;
}
