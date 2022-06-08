from bcc import BPF


text="""
#include <uapi/linux/ptrace.h>
#include <linux/sched.h>

int syscall__execve(struct pt_regs *ctx, const char __user *filename,
        const char __user *const __user *__argv,
        const char __user *const __user *__envp)
{
    char name[256];
    int i = bpf_probe_read(name, sizeof(name), filename);
    bpf_trace_printk("i = %d, %s\\n", i, name);

    return 0;
}

int ret_sys__execve(struct pt_regs *ctx)
{
    return 0;
}
"""

b = BPF(text=text,debug=4)
b.attach_kprobe(event=b.get_syscall_fnname('execve'), fn_name="syscall__execve")
b.attach_kretprobe(event=b.get_syscall_fnname('execve'), fn_name="ret_sys__execve")
b.trace_print()
