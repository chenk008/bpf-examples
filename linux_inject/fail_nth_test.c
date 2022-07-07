#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main()
{
      int i, err, res, fail_nth, fds[2];
      char buf[128];

      system("echo N > /sys/kernel/debug/failslab/ignore-gfp-wait");
      sprintf(buf, "/proc/self/task/%ld/fail-nth", syscall(SYS_gettid));
      fail_nth = open(buf, O_RDWR);
      for (i = 1;; i++) {
              sprintf(buf, "%d", i);
              printf("write %d\n",i);
              write(fail_nth, buf, strlen(buf));
              res = socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
              err = errno;
              pread(fail_nth, buf, sizeof(buf), 0);
              if (res == 0) {
                      close(fds[0]);
                      close(fds[1]);
              }
              printf("%d-th fault %c: res=%d/%d\n", i, atoi(buf) ? 'N' : 'Y',
                      res, err);
              if (atoi(buf))
                      break;
      }
      return 0;
}