#Makefile

obj-m += __module_address.o
CUR_PATH := $(shell pwd)
LINUX_KERNEL := $(shell uname -r)
LINUX_KERNEL_PATH := /usr/src/linux-source-4.4.0/linux-source-4.4.0
all:
	make -C $(LINUX_KERNEL_PATH) M=$(CUR_PATH) modules
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CUR_PATH) clean 

