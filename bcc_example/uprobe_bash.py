#!/usr/bin/python3

import time
from bcc import BPF
from time import sleep


# load BPF program
bpf_text="""
#include <linux/sched.h>

int printForRoot(struct pt_regs *ctx){

    char command[16] = {};

    //use a bpf helper to get the user id.
    uid_t uid = bpf_get_current_uid_gid() & 0xffffffff;

    //another bpf helper to read a string in userland
    bpf_probe_read_user_str(&command, sizeof(command), (void *)PT_REGS_RC(ctx));

    if(uid == 0){
        bpf_trace_printk("Command from root: %s",command);
    }
    return 0;
}
"""


b = BPF(text=bpf_text)
b.attach_uretprobe(name="/bin/bash", sym="readline", fn_name="printForRoot")

while(1):
    try:
        (task, pid, cpu, flags, ts, msg) = b.trace_fields()
    except ValueError as e:
        continue
        printb(b"%-18.9f %s" % (ts, e))
    except KeyboardInterrupt:
        exit()
    

    print("b%s %s" % (time.strftime("%H:%M:%S.%f"), msg))
