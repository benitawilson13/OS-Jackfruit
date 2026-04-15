#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>

#define DEVICE_NAME "container_monitor"
#define CLASS_NAME  "monitor"

static int    majorNumber;
static struct class*  monitorClass  = NULL;
static struct device* monitorDevice = NULL;

static ssize_t monitor_write(struct file *file,
                             const char __user *buffer,
                             size_t len,
                             loff_t *offset)
{
    char kbuf[32];
    long pid;

    if (len > 31)
        len = 31;

    if (copy_from_user(kbuf, buffer, len))
        return -EFAULT;

    kbuf[len] = '\0';

    if (kstrtol(kbuf, 10, &pid) < 0)
        return -EINVAL;

    printk(KERN_INFO "container_monitor: received pid %ld\n", pid);

    return len;
}

static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .write = monitor_write,
};

static int __init monitor_init(void)
{
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0)
    {
        printk(KERN_ALERT "Failed to register major number\n");
        return majorNumber;
    }

    monitorClass = class_create(CLASS_NAME);
    if (IS_ERR(monitorClass))
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        return PTR_ERR(monitorClass);
    }

    monitorDevice = device_create(monitorClass, NULL,
                                 MKDEV(majorNumber, 0),
                                 NULL, DEVICE_NAME);

    printk(KERN_INFO "container_monitor loaded\n");
    return 0;
}

static void __exit monitor_exit(void)
{
    device_destroy(monitorClass, MKDEV(majorNumber, 0));
    class_unregister(monitorClass);
    class_destroy(monitorClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_INFO "container_monitor unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Container Monitor");
