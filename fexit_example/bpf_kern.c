#define __x86_64__
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

SEC("fexit/unix_stream_recvmsg")
int BPF_PROG(read_exit, struct pt_regs *regs, long ret)
{
    u32 pid = bpf_get_current_pid_tgid() >> 32;
    if (pid != 1) {
        return 0;
    }

    // 1. Read in data returned from kernel
    char buffer[100];
    bpf_probe_read_user(
        &buffer, sizeof(buffer), PT_REGS_PARM2(regs));
    bpf_printk("readed is %s\n", buffer); 

    // 3. Overwrite
    // char payload[]="";
    // fexit 不支持 bpf_probe_write_user
    // bpf_probe_write_user(
    //     PT_REGS_PARM2(regs), &payload, sizeof(payload));
    
    // bpf_override_return(regs, sizeof(payload));
    return 0;
}

char __license[] __attribute__((section("license"), used)) = "GPL";