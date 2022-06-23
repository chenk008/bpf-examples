#include "bpf_load.h"
#include <stdio.h>
#include <unistd.h>

#include <linux/bpf.h>
//#include <sys/resource.h>

int main(int argc, char **argv) {
#ifdef __x86_64__
        printf("__x86_64__  is defined\n");
#endif
    if (load_bpf_file("bpf_kern.o")) {
        printf("failed\n");
        printf("%s", bpf_log_buf);
        return 1;
    }
    sleep(1000);
    return 0;
}
