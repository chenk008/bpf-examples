cmd_/root/bpf-examples/modules/deadlock/deadlock.ko := ld -r -m elf_x86_64  --build-id=sha1  -T scripts/module.lds -o /root/bpf-examples/modules/deadlock/deadlock.ko /root/bpf-examples/modules/deadlock/deadlock.o /root/bpf-examples/modules/deadlock/deadlock.mod.o;  true
