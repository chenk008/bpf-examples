CONFIG_MODULE_SIG = n
TARGET_MODULE := test_mutex

obj-m := $(TARGET_MODULE).o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: client
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) client out *.out test
load:
	sudo insmod $(TARGET_MODULE).ko
unload:
	sudo rmmod $(TARGET_MODULE) || true >/dev/null

client:
	gcc -o test client_mtt.c -lpthread

check: all
	$(MAKE) unload
	$(MAKE) load
	sudo ./test
	$(MAKE) unload