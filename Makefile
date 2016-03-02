#Makefile

obj-m += waitqueue_mod.o
CUR_PATH := $(shell pwd)
LINUX_KERNEL := $(shell uname -r)
LINUX_KERNEL_PATH := /usr/src/kernels/linux-${LINUX_KERNEL}
all:
	make -C $(LINUX_KERNEL_PATH) M=$(CUR_PATH) modules
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CUR_PATH) clean 

