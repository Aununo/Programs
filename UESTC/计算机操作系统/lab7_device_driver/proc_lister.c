#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/init_task.h>

#define DEVICE_NAME "proc_lister"
#define CLASS_NAME  "proc_class"

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Gemini Assistant");
MODULE_DESCRIPTION("实验七：Linux设备文件与驱动程序 - 进程列表查看器");

static dev_t dev_num;
static struct class* dev_class = NULL;
static struct cdev my_cdev;

static void print_process_list(void) {
    struct task_struct *p;
    printk(KERN_INFO "============ Process List ============\n");
    printk(KERN_INFO "PID\tSTATE\tPRIO\tPPID\tNAME\n");

    for_each_process(p) {
        printk(KERN_INFO "%d\t%u\t%d\t%d\t%s\n",
               p->pid,
               p->__state,
               p->prio,
               p->real_parent->pid,
               p->comm);
    }
    printk(KERN_INFO "======================================\n");
}

static int my_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "驱动：open() 函数被调用。\n");
    print_process_list();
    return 0;
}

static int my_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "驱动：release() (close) 函数被调用。\n");
    return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "驱动：read() 函数被调用。\n");
    print_process_list();
    return 0;
}

static ssize_t my_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "驱动：write() 函数被调用。\n");
    print_process_list();
    return len;
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    printk(KERN_INFO "驱动：ioctl() 函数被调用。\n");
    print_process_list();
    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
    .unlocked_ioctl = my_ioctl,
};

static int __init proc_lister_init(void) {
    printk(KERN_INFO "正在加载进程列表驱动模块...\n");

    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "错误：无法分配主设备号\n");
        return -1;
    }

    dev_class = class_create(CLASS_NAME);
    if (IS_ERR(dev_class)) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "错误：无法创建设备类\n");
        return PTR_ERR(dev_class);
    }

    if (device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME) == NULL) {
        class_destroy(dev_class);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "错误：无法创建设备文件\n");
        return -1;
    }

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

static void __exit proc_lister_exit(void) {
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "进程列表驱动模块已卸载。\n");
}

module_init(proc_lister_init);
module_exit(proc_lister_exit);