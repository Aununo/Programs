# Makefile for proc_lister kernel module

# 目标模块名称，内核会根据它找到 proc_lister.c 文件 [cite: 74]
obj-m += proc_lister.o

# 内核源码树（或头文件）的路径 [cite: 76]
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# 默认目标，用于编译模块 [cite: 78, 79]
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# 清理目标，用于删除编译生成的文件 [cite: 81]
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean