#!/usr/bin/python
#
# This is a Hello World example that formats output as fields.

from datetime import datetime
from bcc import BPF
from bcc.utils import printb

# define BPF program
prog = """
int hello(void *ctx) {
    bpf_trace_printk("Hello, World!\\n");
    return 0;
}
"""

# load BPF program
b = BPF(text=prog)
b.attach_kprobe(event="calc_global_nohz", fn_name="hello")

# header
print("%-18s %s" % ("TIME(s)", "MESSAGE"))

# format output
while 1:
    try:
        (task, pid, cpu, flags, ts, msg) = b.trace_fields()
    except ValueError as e:
        continue
        printb(b"%-18.9f %s" % (ts, e))
    except KeyboardInterrupt:
        exit()
    

    print(datetime.now().strftime("%H:%M:%S.%f"))
    # printb("%s %s" % (time.strftime("%H:%M:%S.%f"), msg))