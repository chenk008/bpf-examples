#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

SEC("kprobe/__x64_sys_execve")
int kretprobe_sys_read(struct pt_regs *ctx)
{
    char m[]="hello wor";
    bpf_trace_printk(m,sizeof(m));
    return 0;
}

char __license[] __attribute__((section("license"), used)) = "GPL";
