#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

#define FIB_DEV "/dev/test_mutex"
int s = 0;

void child(void *data) {

    int fd = open(FIB_DEV, O_RDWR);
    int tmp = ++s;

    if (fd < 0) {
        perror("Failed to open character device");
        return ;
    }
    printf("%d: Writing %s to kernel\n", tmp, (char*)data);
    write(fd, (char*)data, strlen((char*)data));

    usleep(10);

    char buf[100];
    read(fd, buf, 100);
    printf("%d: Received %s from kernel (Should receive %s)\n", tmp, buf, (char*)data);
}

int main()
{
    pthread_t t[10];
    char strings[][10] = {"aaa", "bbb", "CCC", "ddd", "eee", "fff", "ggg", "hhh", "iii", "jjj"};

    for (int i=0; i<10; ++i) {
        pthread_create(&(t[i]), NULL, (void*)child, strings[i]);
    }

    for (int i=0; i<10; ++i) {
        pthread_join(t[i], NULL);
    }

    return 0;
}