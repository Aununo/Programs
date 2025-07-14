/*
 * proc_lister.c - 一个遵照实验指导实现的字符设备驱动
 */

// 包含实验指导中提到的基本头文件 [cite: 21, 22, 23]
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

// 包含字符设备驱动所需的头文件
#include <linux/fs.h>      // 用于 file_operations 结构 [cite: 145]
#include <linux/cdev.h>    // 用于 cdev 结构 [cite: 153]
#include <linux/device.h>  // 用于 class_create, device_create 等函数 [cite: 222, 223]
#include <linux/sched.h>   // 用于 task_struct 结构 
#include <linux/sched/signal.h> // 用于 for_each_process 宏
#include <linux/init_task.h> // 用于 init_task 符号 

#define DEVICE_NAME "proc_lister"
#define CLASS_NAME  "proc_class"

// 模块许可和信息
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Gemini Assistant");
MODULE_DESCRIPTION("实验七：Linux设备文件与驱动程序 - 进程列表查看器");

// 驱动所需变量
static dev_t dev_num;           // 设备号 [cite: 168]
static struct class* dev_class = NULL; // 设备类
static struct cdev my_cdev;     // 字符设备结构 [cite: 153, 167]

// 实验核心功能：遍历并打印进程信息 
static void print_process_list(void) {
    struct task_struct *p; // [cite: 232]
    printk(KERN_INFO "============ Process List ============\n");
    printk(KERN_INFO "PID\tSTATE\tPRIO\tPPID\tNAME\n");

    // 使用 for_each_process 宏遍历所有进程 [cite: 234]
    for_each_process(p) {
        printk(KERN_INFO "%d\t%ld\t%d\t%d\t%s\n",
               p->pid,
               p->__state,
               p->prio,
               p->real_parent->pid, // 获取父进程ID
               p->comm); // 进程名
    }
    printk(KERN_INFO "======================================\n");
}

// 实现 file_operations 结构中的方法 [cite: 204, 205, 206]
static int my_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "驱动：open() 函数被调用。\n");
    print_process_list(); // 遵照实验要求，在操作时触发打印
    return 0;
}

static int my_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "驱动：release() (close) 函数被调用。\n");
    return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "驱动：read() 函数被调用。\n");
    print_process_list();
    // 仅打印内核日志，不向用户空间返回数据，返回0表示读取结束。
    return 0;
}

static ssize_t my_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "驱动：write() 函数被调用。\n");
    print_process_list();
    // 假设写入成功，返回写入的长度以避免应用层出错。
    return len;
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    printk(KERN_INFO "驱动：ioctl() 函数被调用。\n");
    print_process_list();
    return 0;
}

// 定义 file_operations 结构，并将其实例化 [cite: 145, 176]
static const struct file_operations fops = {
    .owner = THIS_MODULE,        // 指向模块自身 [cite: 178]
    .open = my_open,             // 对应 open 系统调用 [cite: 182, 188]
    .release = my_release,       // 对应 close 系统调用 [cite: 183, 190]
    .read = my_read,             // 对应 read 系统调用 [cite: 179, 194]
    .write = my_write,           // 对应 write 系统调用 [cite: 180, 192]
    .unlocked_ioctl = my_ioctl,  // 对应 ioctl 系统调用 [cite: 181, 196]
};

// 模块初始化函数 [cite: 207]
static int __init proc_lister_init(void) {
    printk(KERN_INFO "正在加载进程列表驱动模块...\n");

    // 步骤1: 动态分配设备号 [cite: 170, 221]
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "错误：无法分配主设备号\n");
        return -1;
    }

    // 步骤2: 创建设备类 [cite: 222]
    dev_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(dev_class)) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "错误：无法创建设备类\n");
        return PTR_ERR(dev_class);
    }

    // 步骤3: 创建设备文件（节点） [cite: 220, 223]
    if (device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME) == NULL) {
        class_destroy(dev_class);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "错误：无法创建设备文件\n");
        return -1;
    }

    // 步骤4: 初始化并注册cdev结构 [cite: 216, 217, 218, 219]
    cdev_init(&my_cdev, &fops);
    if (cdev_add(&my_cdev, dev_num, 1) < 0) {
        device_destroy(dev_class, dev_num);
        class_destroy(dev_class);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "错误：无法向内核注册cdev\n");
        return -1;
    }

    printk(KERN_INFO "进程列表驱动模块加载成功！设备文件位于 /dev/%s\n", DEVICE_NAME);
    return 0;
}

// 模块退出函数 [cite: 211]
static void __exit proc_lister_exit(void) {
    // 卸载过程与加载过程相反 [cite: 212, 213, 214]
    device_destroy(dev_class, dev_num);      // 删除设备文件 [cite: 225]
    class_destroy(dev_class);                // 删除设备类 [cite: 226]
    cdev_del(&my_cdev);                      // 从内核删除cdev [cite: 227]
    unregister_chrdev_region(dev_num, 1);    // 释放设备号 [cite: 228]
    printk(KERN_INFO "进程列表驱动模块已卸载。\n");
}

// 申明模块的初始化和退出函数 [cite: 37, 39]
module_init(proc_lister_init);
module_exit(proc_lister_exit);