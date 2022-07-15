#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout << "error parameter" << endl;
        exit(-1);
    }
    char *filename = argv[1];
    int fd = open(filename, O_RDWR, 0644);
    struct stat s;
    if (fstat(fd, &s) < 0)
    {
        close(fd);
        return -1;
    }
    int prot = 0;
    char *prot_str = argv[2];
    if (strstr(prot_str, "PROT_READ") != NULL)
    {
        prot |= PROT_READ;
    }
    if (strstr(prot_str, "PROT_WRITE") != NULL)
    {
        prot |= PROT_WRITE;
    }
    int flag = 0;
    char *flag_str = argv[3];
    if (strstr(flag_str, "MAP_SHARED") != NULL)
    {
        flag |= MAP_SHARED;
    }
    if (strstr(flag_str, "MAP_PRIVATE") != NULL)
    {
        flag |= MAP_PRIVATE;
    }
    if (strstr(flag_str, "MAP_POPULATE") != NULL)
    {
        flag |= MAP_POPULATE;
    }
    if (strstr(flag_str, "MAP_LOCKED") != NULL)
    {
        flag |= MAP_LOCKED;
    }
    char *base = (char *)mmap(0, s.st_size, prot, flag, fd, 0); /*    volatile char ch;    for(int i=0; i<s.st_size; i+=4096){//测试时进行读操作        ch = base[i];    }    for(int i=0; i<s.st_size; i+=4096){//测试时进行写操作        base[i] = '\0';    }    */
    cout << "mmap region 0x" << hex << (long)base << " " << strerror(errno) << endl;
    cout << base << endl;
    base[0]='3';
    cout << base << endl;

    sleep(1000);
    munmap(base, s.st_size);
    close(fd);
    return 0;
}
