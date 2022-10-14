#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>  
#include <linux/init.h> 
#include <linux/ioctl.h> 
#include <linux/slab.h>
#include <linux/poll.h>

#define DEVICE_NAME "qrng"

static ssize_t qrng_read_iter(struct kiocb*, struct iov_iter*);
static ssize_t qrng_write_iter(struct kiocb*, struct iov_iter*);
static __poll_t qrng_poll(struct file*, poll_table*);
static long qrng_ioctl(struct file*, unsigned int cmd, unsigned long arg);
static int qrng_fasync(int, struct file*, int);

const struct file_operations qrng_fops = {
    /* implemented functions */
    .read_iter = qrng_read_iter,
    .write_iter = qrng_write_iter,
    .poll = qrng_poll,
    .unlocked_ioctl = qrng_ioctl,
    .fasync = qrng_fasync,

    /* non-implemented functions */
    .compat_ioctl = compat_ptr_ioctl,
    .llseek = noop_llseek,
    .splice_read = generic_file_splice_read,
    .splice_write = iter_file_splice_write,
};

static int major;

static int __init qrng_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &qrng_fops);

    if (major < 0) {
        printk(KERN_ALERT "QRNG service load failed\n");
        return major;
    }

    printk(KERN_INFO "QRNG service module has been loaded: %d\n", major);
    return 0;
}

static void __exit qrng_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "QRNG service module has been unloaded\n");
}

static ssize_t qrng_write_iter(struct kiocb*, struct iov_iter*) {
   printk(KERN_INFO "Sorry, QRNG service is read only\n");
   return -EFAULT;
}


module_init(qrng_init);
module_exit(qrng_exit);

MODULE_LICENSE("GPL");