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

#define QRNG_DEBUG

#define DEVICE_NAME "qrandom0"
#define BUFF_BLOCK  100 // requested byte count from qrng.lumii.lv
#define BLOCK_CNT 4
#define BUFF_TOTAL (BUFF_BLOCK*BLOCK_CNT)

static u8 qrng_buffer[BUFF_TOTAL];
static bool buff_is_renewed[BLOCK_CNT];
static int  buff_read_it; // iterates [0;BUFF_TOTAL)
static int  block_write_it; // iterates blocks [0:BLOCK_CNT)
static int  buff_ready_cnt = 0;

static bool qrng_ready(void) {
    int buff_id;

    if(buff_read_it%BUFF_BLOCK) return true;
    buff_id = buff_read_it/BUFF_BLOCK;
    return buff_is_renewed[buff_id];
}

static ssize_t get_random_byte(u8* b) {
    int buff_id = buff_read_it/BUFF_BLOCK;
    if(buff_read_it%BUFF_BLOCK==0) {
        if(!buff_is_renewed[buff_id]) return 0;
        buff_ready_cnt--;
        buff_is_renewed[buff_id] = false;
    }
    *b = qrng_buffer[buff_read_it];
    qrng_buffer[buff_read_it] = 0;
    buff_read_it = (buff_read_it+1)%BUFF_TOTAL;
    return 1;
}

static ssize_t get_random_bytes_qrng(struct iov_iter* iter) {
    size_t ret = 0;

    if(unlikely(!iov_iter_count(iter)))
        return 0;

    while(iov_iter_count(iter)) {
        u8 b;
        ssize_t r = get_random_byte(&b);
        if(r==0) break;
        ret += copy_to_iter(&b,sizeof(b),iter);
    }
    
    return ret ? ret : -EFAULT;
}


static ssize_t qrng_read_iter(struct kiocb*, struct iov_iter*);
static ssize_t qrng_write_iter(struct kiocb*, struct iov_iter*);
static __poll_t qrng_poll(struct file*, poll_table*);
// static long qrng_ioctl(struct file*, unsigned int cmd, unsigned long arg);
static int qrng_fasync(int, struct file*, int);

const struct file_operations qrng_fops = {
    /* implemented functions */
    .read_iter = qrng_read_iter,
    .write_iter = qrng_write_iter,
    .poll = qrng_poll,
    //.unlocked_ioctl = qrng_ioctl,
    .fasync = qrng_fasync,

    /* non-implemented functions */
    .compat_ioctl = compat_ptr_ioctl,
    .llseek = noop_llseek,
    .splice_read = generic_file_splice_read,
    .splice_write = iter_file_splice_write,
};

static int major;
static struct class* cls;

static int __init qrng_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &qrng_fops);

    if (major < 0) {
        printk(KERN_ALERT "QRNG service load failed\n");
        return major;
    }

    printk(KERN_INFO "QRNG service module has been loaded: %d\n", major);

    cls = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(cls, NULL, MKDEV(major,0),NULL,DEVICE_NAME);

    pr_info("Device created on /dev/%s\n", DEVICE_NAME);

    return 0;
}

static void __exit qrng_exit(void) {
    device_destroy(cls, MKDEV(major, 0)); 
    class_destroy(cls); 

    unregister_chrdev(major, DEVICE_NAME);

    printk(KERN_INFO "QRNG service module has been unloaded\n");
}


static ssize_t qrng_read_iter(struct kiocb* kiocb, struct iov_iter* iter) {
    #ifdef QRNG_DEBUG
        pr_info("QRNG: read_iter function called\n");
    #endif
    if(!qrng_ready())
        return -EAGAIN;

    return get_random_bytes_qrng(iter);
}

static ssize_t qrng_write_iter(struct kiocb*, struct iov_iter* iter) {

    ssize_t ret = 0;
    size_t copied;

    if(unlikely(!iov_iter_count(iter)))
        return 0;

    #ifdef QRNG_DEBUG
        pr_info("QRNG: write_iter function called\n");
        pr_info("QRNG: write_iter iov_iter_count %d\n",iov_iter_count(iter));
    #endif

    while(iov_iter_count(iter)>=BUFF_BLOCK&&buff_ready_cnt<BLOCK_CNT) {
        size_t block_sz = sizeof(qrng_buffer[0])*BLOCK_SIZE;
        void* block_addr = &qrng_buffer[BLOCK_SIZE*block_write_it];
        copied = copy_from_iter(block_addr, block_sz, iter);
        if(copied==BUFF_BLOCK)
        {
            ret += copied;
            buff_is_renewed[block_write_it] = 1;
            block_write_it = (block_write_it+1)%BLOCK_CNT;
            buff_ready_cnt++;
        }
    }
    #ifdef QRNG_DEBUG
        pr_info("QRNG: write_iter ret %d\n",ret);
    #endif
    
    return ret ? ret : -EFAULT;
}

static __poll_t qrng_poll(struct file* file, poll_table* wait) {
    #ifdef QRNG_DEBUG
        pr_info("QRNG: poll function called\n");
    #endif
    __poll_t res = 0;
    if(buff_ready_cnt<BUFF_BLOCK) res |= EPOLLOUT;  // writing is now possible
    if(qrng_ready()) res |= POLLIN|EPOLLRDNORM;     // there is data to read.
    return res;
}


static struct fasync_struct *fasync;
static int qrng_fasync(int fd, struct file *filp, int on) {
    return fasync_helper(fd, filp, on, &fasync);
}

module_init(qrng_init);
module_exit(qrng_exit);

MODULE_LICENSE("GPL");