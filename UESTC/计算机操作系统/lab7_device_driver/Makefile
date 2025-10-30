# Makefile for proc_lister kernel module

# 目标模块名称，内核会根据它找到 proc_lister.c 文件 
obj-m += proc_lister.o

# 内核源码树（或头文件）的路径 
# WSL2 默认内核没有 build 目录，使用已安装的内核头文件
KDIR := /lib/modules/6.8.0-86-generic/build
PWD := $(shell pwd)

# 默认目标，用于编译模块 
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# 清理目标，用于删除编译生成的文件
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean