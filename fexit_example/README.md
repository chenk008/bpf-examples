验证bpf是不是可用：
bpftool prog load bpf_kern.o /sys/fs/bpf/test

查看section
readelf -S bpf_kern.o