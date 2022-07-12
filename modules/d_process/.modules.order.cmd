cmd_/root/bpf-examples/modules/d_process/modules.order := {   echo /root/bpf-examples/modules/d_process/mock.ko; :; } | awk '!x[$$0]++' - > /root/bpf-examples/modules/d_process/modules.order
