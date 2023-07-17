#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/uio.h>
#include <linux/mutex.h>

#define DEVICE_NAME "qrandom0"
#define BUFF_SIZE 1024

static u8 qrng_buffer[BUFF_SIZE];
static u8 qrng_buff_ready[BUFF_SIZE]; // true or false
static int qrng_buff_read_it = 0;     // iterates
static int qrng_buff_write_it = 0;

#define read_ready (qrng_buff_ready[qrng_buff_read_it])
#define write_ready (!qrng_buff_ready[qrng_buff_write_it])

static DEFINE_MUTEX(buff_mutex);
static DECLARE_WAIT_QUEUE_HEAD(read_queue); // for processes that want to read
static DECLARE_WAIT_QUEUE_HEAD(poll_queue); // for processes that might want to read or write

static ssize_t get_random_byte(u8 *b)
{
    mutex_lock(&buff_mutex);
    if (mutex_is_locked(&buff_mutex) == 0)
        return 0;

    if (!read_ready)
    {
        mutex_unlock(&buff_mutex);
        return 0;
    }

    qrng_buff_ready[qrng_buff_read_it] = false;              // mark byte as unavailable
    *b = qrng_buffer[qrng_buff_read_it];                     // read byte at iterator
    qrng_buffer[qrng_buff_read_it] = 0;                      // clear its value
    qrng_buff_read_it = (qrng_buff_read_it + 1) % BUFF_SIZE; // increment read iterator

    mutex_unlock(&buff_mutex);

    wake_up_interruptible(&poll_queue); // a byte can be renewed
    if (read_ready)
        wake_up_interruptible(&read_queue); // a byte can be read

    return 1; // the function always reads just one byte
}

static ssize_t write_random_bytes_qrng(struct iov_iter *iter)
{
    ssize_t ret = 0;

    mutex_lock(&buff_mutex);
    if (mutex_is_locked(&buff_mutex) == 0)
        return 0;

    if (!write_ready)
    {
        mutex_unlock(&buff_mutex);
        return 0;
    }

    while (write_ready && iov_iter_count(iter))
    {
        u8 byte;
        ssize_t copied = copy_from_iter(&byte, sizeof(byte), iter);
        if (copied != sizeof(byte))
            break;
        ret += copied;
        qrng_buffer[qrng_buff_write_it] = byte;
        qrng_buff_ready[qrng_buff_write_it] = 1;
        qrng_buff_write_it = (qrng_buff_write_it + 1) % BUFF_SIZE;
    }

    mutex_unlock(&buff_mutex);

    if (ret)
    {
        wake_up_interruptible(&read_queue);
        wake_up_interruptible(&poll_queue);
    }

    return ret;
}

static ssize_t get_random_bytes_qrng(struct iov_iter *iter, bool get_all)
{
    size_t ret = 0;

    if (unlikely(!iov_iter_count(iter)))
        return 0;

    while (iov_iter_count(iter))
    {
        u8 b;
        ssize_t r = get_random_byte(&b);
        if (r == 0)
        {
            if (get_all)
            {
                if (wait_event_interruptible(read_queue, read_ready) != 0)
                    return -ERESTARTSYS;
                continue;
            }
            else return ret;
        }
        ret += copy_to_iter(&b, sizeof(b), iter);
    }

    return ret ? ret : -EFAULT;
}

static ssize_t qrng_read_iter(struct kiocb *, struct iov_iter *);
static ssize_t qrng_write_iter(struct kiocb *, struct iov_iter *);
static __poll_t qrng_poll(struct file *, poll_table *);
static int qrng_fasync(int, struct file *, int);

const struct file_operations qrng_fops = {
    /* implemented functions */
    .read_iter = qrng_read_iter,
    .write_iter = qrng_write_iter,
    .poll = qrng_poll,
    .fasync = qrng_fasync,

    /* non-implemented functions */
    .compat_ioctl = compat_ptr_ioctl,
    .llseek = noop_llseek,
    .splice_read = generic_file_splice_read,
    .splice_write = iter_file_splice_write,
};

static int major;
static struct class *cls;

static char *qrng_devnode(const struct device *dev, umode_t *mode)
{
    if (mode) *mode = 0644;
    return NULL;
}

static int __init qrng_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &qrng_fops);

    if (major < 0)
    {
        printk(KERN_ALERT "QRNG service load failed\n");
        return major;
    }

    printk(KERN_INFO "QRNG service module has been loaded: %d\n", major);

    cls = class_create(DEVICE_NAME);
    cls->devnode = qrng_devnode;
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

    pr_info("Device created on /dev/%s\n", DEVICE_NAME);

    return 0;
}

static void __exit qrng_exit(void)
{
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);

    unregister_chrdev(major, DEVICE_NAME);

    printk(KERN_INFO "QRNG service module has been unloaded\n");
}

static ssize_t qrng_read_iter(struct kiocb *kiocb, struct iov_iter *iter)
{
    bool nonblocking = false;

    nonblocking |= (kiocb->ki_flags & (IOCB_NOWAIT | IOCB_NOIO));
    nonblocking |= (kiocb->ki_filp->f_flags & O_NONBLOCK);

    if (!read_ready && nonblocking)
        return -EAGAIN;

    return get_random_bytes_qrng(iter, !nonblocking);
}

static ssize_t qrng_write_iter(struct kiocb *, struct iov_iter *iter)
{

    ssize_t ret = 0;
    size_t copied;

    if (unlikely(!iov_iter_count(iter)))
        return 0;

    while (iov_iter_count(iter) && write_ready)
    {
        copied = write_random_bytes_qrng(iter);
        if (copied == 0)
            break;
        ret += copied;
    }

    return ret ? ret : -EFAULT;
}

static __poll_t qrng_poll(struct file *file, poll_table *wait)
{
    __poll_t res = 0;

    poll_wait(file, &poll_queue, wait);

    if (write_ready)
        res |= EPOLLOUT; // writing is now possible
    if (read_ready)
        res |= POLLIN | EPOLLRDNORM; // there is data to read.

    return res;
}

static struct fasync_struct *fasync;
static int qrng_fasync(int fd, struct file *filp, int on)
{
    return fasync_helper(fd, filp, on, &fasync);
}

module_init(qrng_init);
module_exit(qrng_exit);

MODULE_LICENSE("GPL");
