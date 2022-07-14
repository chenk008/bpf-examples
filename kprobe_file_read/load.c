#include "bpf_load.h"
#include <stdio.h>
#include <unistd.h>
#include <bpf/bpf.h>
#include "my.h"

#include <linux/bpf.h>
//#include <sys/resource.h>

int main(int argc, char **argv) {
    if (argc < 2){
        printf("the comm shoule be specify");
        return 1;
    } 
#ifdef __x86_64__
        printf("__x86_64__  is defined\n");
#endif
    if (load_bpf_file("bpf_test.o")) {
        printf("%s", bpf_log_buf);
        return 1;
    }
    int i=1;
    printf("the comm is %s\n",argv[1]);
    struct procName pn;
    bzero(&pn, sizeof(struct procName));
    strcpy(pn.name,argv[1]);
    if (bpf_map_update_elem(map_fd[0], &pn, &i , BPF_ANY)){
        printf("failed to update map\n");
    }
    printf("sleeping\n");
    sleep(1000);
    return 0;
}
