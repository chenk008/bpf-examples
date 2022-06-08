from bcc import BPF


text="""
#include <uapi/linux/ptrace.h>
#include <linux/sched.h>

int syscall__probe_entry_sendto(struct pt_regs* ctx, int sockfd, char* buf, size_t len, int flags,
                                const struct sockaddr* dest_addr, size_t addrlen) {
    bpf_trace_printk("bcc - entry_sendto - 0 %p", ctx);
    bpf_trace_printk("bcc - entry_sendto - 1 %p", PT_REGS_PARM1(ctx));
    bpf_trace_printk("bcc - entry_sendto - 2 %p", PT_REGS_PARM2(ctx));
    bpf_trace_printk("bcc - entry_sendto - 3 %p", PT_REGS_PARM3(ctx));
    bpf_trace_printk("bcc - entry_sendto - 1 %d", (int)PT_REGS_PARM1(ctx));
    bpf_trace_printk("bcc - entry_sendto - 2 %s", (char *)PT_REGS_PARM2(ctx));
    bpf_trace_printk("bcc - entry_sendto - 3 %d", (int)PT_REGS_PARM3(ctx));
    bpf_trace_printk("bcc - entry_sendto params - 1 %d", sockfd);
    bpf_trace_printk("bcc - entry_sendto params - 2 %s", buf);
    bpf_trace_printk("bcc - entry_sendto params - 3 %d", len);
    return 0;
}
"""

b = BPF(text=text,debug=4)
b.attach_kprobe(event=b.get_syscall_fnname('sendto'), fn_name="syscall__probe_entry_sendto")
b.trace_print()