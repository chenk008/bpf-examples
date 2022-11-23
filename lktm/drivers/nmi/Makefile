#
### Makefile for Linux sample_nmi
#
#obj-m += sample_nmi.o

# sampleblk-objs := sample_nmi.o
obj-m:=sample_nmi.o
CURRENT_PATH:=$(shell pwd)
LINUX_KERNEL:=$(shell uname -r)
LINUX_KERNEL_PATH:=/lib/modules/$(shell uname -r)/build
all:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean