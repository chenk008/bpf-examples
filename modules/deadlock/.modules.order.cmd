cmd_/root/bpf-examples/modules/deadlock/modules.order := {   echo /root/bpf-examples/modules/deadlock/deadlock.ko; :; } | awk '!x[$$0]++' - > /root/bpf-examples/modules/deadlock/modules.order
