obj-m := kprobe_example.o

CROSS_COMPILE=''

KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.mod.o *.mod.c .*.cmd *.symvers  modul*