#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>

#include "monitor_ioctl.h"

#define DEVICE_NAME "monitor"

static dev_t dev_num;
static struct cdev monitor_cdev;
static struct class *monitor_class;

static long monitor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct container_info ci;

    if (copy_from_user(&ci, (void __user *)arg, sizeof(ci)))
        return -EFAULT;

    switch (cmd) {
    case MONITOR_IOC_REGISTER:
        printk(KERN_INFO "monitor: register pid=%d name=%s\n",
               ci.pid, ci.name);
        break;

    case MONITOR_IOC_UNREGISTER:
        printk(KERN_INFO "monitor: unregister pid=%d name=%s\n",
               ci.pid, ci.name);
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

static int monitor_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int monitor_release(struct inode *inode, struct file *file)
{
    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = monitor_open,
    .release = monitor_release,
    .unlocked_ioctl = monitor_ioctl,
};

static int __init monitor_init(void)
{
    alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);

    cdev_init(&monitor_cdev, &fops);
    cdev_add(&monitor_cdev, dev_num, 1);

    monitor_class = class_create("monitor_class");
    device_create(monitor_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "monitor: module loaded\n");
    return 0;
}

static void __exit monitor_exit(void)
{
    device_destroy(monitor_class, dev_num);
    class_destroy(monitor_class);
    cdev_del(&monitor_cdev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "monitor: module unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OS-Jackfruit");
MODULE_DESCRIPTION("Simple container monitor");
