bpf_load.o: bpf_load.c
	gcc -I/root/source/bpftool/include -c bpf_load.c

load.o: load.c
	gcc -c load.c

load: load.o bpf_load.o
	gcc -lbpf -lelf load.o bpf_load.o -o load

clean:
	rm load.o bpf_load.o

bpf_kern.o: bpf_kern.c
	clang -g  -O2 -Wall -target bpf -c bpf_kern.c -o bpf_kern.o