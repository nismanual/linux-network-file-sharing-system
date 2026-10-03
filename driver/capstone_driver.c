#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "capstone_device"
#define CLASS_NAME  "capstone"

static int major_number;
static struct class *capstone_class;
static struct device *capstone_device;

static char device_buffer[256];
static size_t buffer_size = 0;

/* Called when a process opens /dev/capstone_device */
static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Capstone Driver: device opened\n");
    return 0;
}

/* Called when a process reads from /dev/capstone_device */
static ssize_t device_read(
    struct file *file,
    char __user *buffer,
    size_t length,
    loff_t *offset)
{
    size_t bytes_to_copy;

    if (*offset >= buffer_size)
        return 0;

    bytes_to_copy = min(length, buffer_size - (size_t)*offset);

    if (copy_to_user(buffer, device_buffer + *offset, bytes_to_copy))
        return -EFAULT;

    *offset += bytes_to_copy;

    printk(KERN_INFO "Capstone Driver: data read\n");

    return bytes_to_copy;
}

/* Called when a process writes to /dev/capstone_device */
static ssize_t device_write(
    struct file *file,
    const char __user *buffer,
    size_t length,
    loff_t *offset)
{
    size_t bytes_to_copy;

    bytes_to_copy = min(length, sizeof(device_buffer) - 1);

    if (copy_from_user(device_buffer, buffer, bytes_to_copy))
        return -EFAULT;

    device_buffer[bytes_to_copy] = '\0';
    buffer_size = bytes_to_copy;

    printk(KERN_INFO "Capstone Driver: data written\n");

    return bytes_to_copy;
}

/* Called when a process closes the device */
static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Capstone Driver: device closed\n");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .read = device_read,
    .write = device_write,
    .release = device_release,
};

static int __init capstone_init(void)
{
    printk(KERN_INFO "Capstone Driver: initializing\n");

    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_number < 0) {
        printk(KERN_ALERT "Capstone Driver: failed to register device\n");
        return major_number;
    }

    capstone_class = class_create(CLASS_NAME);

    if (IS_ERR(capstone_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(capstone_class);
    }

    capstone_device = device_create(
        capstone_class,
        NULL,
        MKDEV(major_number, 0),
        NULL,
        DEVICE_NAME
    );

    if (IS_ERR(capstone_device)) {
        class_destroy(capstone_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(capstone_device);
    }

    printk(KERN_INFO
           "Capstone Driver: registered with major number %d\n",
           major_number);

    return 0;
}

static void __exit capstone_exit(void)
{
    device_destroy(
        capstone_class,
        MKDEV(major_number, 0)
    );

    class_destroy(capstone_class);

    unregister_chrdev(
        major_number,
        DEVICE_NAME
    );

    printk(KERN_INFO "Capstone Driver: unloaded\n");
}

module_init(capstone_init);
module_exit(capstone_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nishi");
MODULE_DESCRIPTION("Capstone Linux Character Device Driver");
MODULE_VERSION("1.0");
