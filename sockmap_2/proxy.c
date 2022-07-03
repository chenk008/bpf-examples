#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <signal.h>

#define MAXSIZE 100

char buf[MAXSIZE];
int proxysd1, proxysd2;

static void int_handler(int a)
{
    close(proxysd1);
    close(proxysd2);
    exit(0);
}

int main(int argc, char *argv[])
{
    int ret;
    struct sockaddr_in proxyaddr1, proxyaddr2;
    struct hostent *proxy1, *proxy2;
    unsigned short port1, port2;
    fd_set rset;
    int maxfd = 10, n;

    if (argc != 5) {
        exit(1);
    }

    signal(SIGINT, int_handler);

    FD_ZERO(&rset);

    proxysd1 = socket(AF_INET, SOCK_STREAM, 0);
    proxysd2 = socket(AF_INET, SOCK_STREAM, 0);

    proxy1 = gethostbyname(argv[1]);
    port1 = atoi(argv[2]);

    proxy2 = gethostbyname(argv[3]);
    port2 = atoi(argv[4]);

    bzero(&proxyaddr1, sizeof(struct sockaddr_in));
    proxyaddr1.sin_family = AF_INET;
    proxyaddr1.sin_port = htons(port1);
    proxyaddr1.sin_addr = *((struct in_addr *)proxy1->h_addr);

    bzero(&proxyaddr2, sizeof(struct sockaddr_in));
    proxyaddr2.sin_family = AF_INET;
    proxyaddr2.sin_port = htons(port2);
    proxyaddr2.sin_addr = *((struct in_addr *)proxy2->h_addr);

    // proxy作为client，分别去connect两个server。
    connect(proxysd1, (struct sockaddr *)&proxyaddr1, sizeof(struct sockaddr));
    connect(proxysd2, (struct sockaddr *)&proxyaddr2, sizeof(struct sockaddr));

    while (1) {
        FD_SET(proxysd1, &rset);
        FD_SET(proxysd2, &rset);
        select(maxfd, &rset, NULL, NULL, NULL);
        memset(buf, 0, MAXSIZE);
        if (FD_ISSET(proxysd1, &rset)) {
            ret = recv(proxysd1, buf, MAXSIZE, 0);
            printf("%d --> %d proxy string:%s\n", proxysd1, proxysd2, buf);
            send(proxysd2, buf, ret, 0);
        }
        if (FD_ISSET(proxysd2, &rset)) {
            ret = recv(proxysd2, buf, MAXSIZE, 0);
            printf("%d --> %d proxy string:%s\n", proxysd2, proxysd1, buf);
            send(proxysd1, buf, ret, 0);
        }
    }

    return 0;
}