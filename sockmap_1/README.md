## 编译

```
$ clang -O2 -g -target bpf -c bpf_sockops.c -o bpf_sockops.o
$ clang -O2 -g -target bpf -c bpf_redir.c -o bpf_redir.o
```

## 加载bpf_sockops到内核
```
$ sudo bpftool prog load bpf_sockops.o /sys/fs/bpf/bpf_sockops type sockops
```
这条命令将 object 代码加载到内核（但还没 attach 到 hook 点）
加载之后的代码会 pin 到一个 BPF 虚拟文件系统 来持久存储，这样就能获得一个指向这个程序的文件句柄（handle）供稍后使用。
bpftool 会在 ELF 目标文件中创建我们声明的 sockmap（sock_ops_map 变量，定 义在头文件中）。

## Attach bpf_sockops 到 cgroups
```
$ sudo bpftool cgroup attach /sys/fs/cgroup/unified/ sock_ops pinned /sys/fs/bpf/bpf_sockops
```
这条命令将加载之后的 sock_ops 程序 attach 到指定的 cgroup，
这个 cgroup 内的所有进程的所有 sockets，都将会应用这段程序。如果使用的是 cgroupv2 时，systemd 会在 /sys/fs/cgroup/unified 自动创建一个 mount 点。


## 加载sk_msg到内核
```
sudo bpftool prog load bpf_redir.o /sys/fs/bpf/bpf_redir \
    map name sock_ops_map \
    pinned /sys/fs/bpf/sock_ops_map
```
将程序加载到内核
将程序 pin 到 BPF 文件系统的 /sys/fs/bpf/bpf_redir 位置
重用已有的 sockmap，指定了 sockmap 的名字为 sock_ops_map 并且文件路径为 /sys/fs/bpf/sock_ops_map

## Attach sk_msg
将已经加载到内核的 sk_msg 程序 attach 到 sockmap
```
$ sudo bpftool prog attach pinned /sys/fs/bpf/bpf_redir msg_verdict pinned /sys/fs/bpf/sock_ops_map
```
从现在开始，sockmap 内的所有 socket 在 sendmsg 时都将触发执行这段 BPF 代码。


## 测试

在一个窗口中启动 socat 作为服务端，监听在 1000 端口：
```
$ sudo socat TCP4-LISTEN:1000,fork exec:cat
```

另一个窗口用 nc 作为客户端来访问服务端，建立 socket：
```
$ nc localhost 1000
```

观察我们在 BPF 代码中打印的日志：
```
$ sudo cat /sys/kernel/debug/tracing/trace_pipe
    nc-13227   [002] .... 105048.340802: 0: sockmap: op 4, port 50932 --> 1001
    nc-13227   [002] ..s1 105048.340811: 0: sockmap: op 5, port 1001 --> 50932
```

## 清理
从 sockmap 中 detach 第二段 BPF 程序，并将其从 BPF 文件系统中 unpin：
```
$ sudo bpftool prog detach pinned /sys/fs/bpf/bpf_redir msg_verdict pinned /sys/fs/bpf/sock_ops_map
$ sudo rm /sys/fs/bpf/bpf_redir
```

同理，从 cgroups 中 detach 第一段 BPF 程序，并将其从 BPF 文件系统中 unpin：
```
$ sudo bpftool cgroup detach /sys/fs/cgroup/unified/ sock_ops pinned /sys/fs/bpf/bpf_sockops
$ sudo rm /sys/fs/bpf/bpf_sockops
```

最后删除 sockmaps：
```
$ sudo rm /sys/fs/bpf/sock_ops_map
```