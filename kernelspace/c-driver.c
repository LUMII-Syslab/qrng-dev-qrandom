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

//#define QRNG_DEBUG

#define DEVICE_NAME "qrandom0"
#define RNG_BLOCK_SZ  (size_t)256 // requested byte count from qrng.lumii.lv
#define RNG_BLOCK_CNT 4
#define RNG_TOTAL (RNG_BLOCK_SZ*RNG_BLOCK_CNT)

static u8   qrng_buffer[RNG_TOTAL];
static bool block_is_renewed[RNG_BLOCK_CNT];
static int  buff_read_it = 0; // iterates [0;RNG_TOTAL)
static int  block_write_it = 0; // iterates blocks [0:RNG_BLOCK_CNT)
static int  buff_ready_cnt = 0;
static bool qrng_ready = false;

static DEFINE_MUTEX(buff_mutex);
DECLARE_WAIT_QUEUE_HEAD (read_queue); // for processes that want to read
DECLARE_WAIT_QUEUE_HEAD (poll_queue); // for processes that might want to read or write

static ssize_t get_random_byte(u8* b) {
    int ret;

    if(!qrng_ready) return 0;

    ret = mutex_trylock(&buff_mutex);
    if(ret!=0){
        if(mutex_is_locked(&buff_mutex)==0) {
            pr_info("QRNG: failed to lock mutex in get_random_byte\n");
            return 0;
        }

        if(!qrng_ready){
            mutex_unlock(&buff_mutex);
            return 0;
        }

        if(buff_read_it%RNG_BLOCK_SZ==0) {
            buff_ready_cnt--;
            wake_up_interruptible(&poll_queue);
            block_is_renewed[buff_read_it/RNG_BLOCK_SZ] = false;
        }

        *b = qrng_buffer[buff_read_it];
        qrng_buffer[buff_read_it] = 0;

        buff_read_it = (buff_read_it+1)%RNG_TOTAL;
        if(buff_read_it%RNG_BLOCK_SZ==0){
            qrng_ready = block_is_renewed[buff_read_it/RNG_BLOCK_SZ];
            if(qrng_ready) {
                wake_up_interruptible(&read_queue);
            }
            wake_up_interruptible(&poll_queue);
        }

        mutex_unlock(&buff_mutex); 
        return 1;
    }else {
        pr_info("QRNG: failed to lock mutex in get_random_byte\n");
        return 0;
    }
    return 0;
}


static ssize_t write_random_bytes_qrng(struct iov_iter* iter) {
    void* block_addr;
    size_t copied;
    
    int ret = mutex_trylock(&buff_mutex);
    if(ret!=0){
        if(mutex_is_locked(&buff_mutex)==0) {
            pr_info("QRNG: failed to lock mutex in write_random_bytes_qrng\n");
            return 0;
        }
        
        block_addr = &qrng_buffer[RNG_BLOCK_SZ*block_write_it];
        copied = copy_from_iter(block_addr, RNG_BLOCK_SZ, iter);
        
        pr_info("QRNG: copied %d bytes in write_random_bytes_qrng\n", (int)copied);
        block_is_renewed[block_write_it] = true;

        // if iterator is at the start of a block that was just written to, mark ready as true
        if(qrng_ready==false&&(buff_read_it%RNG_BLOCK_SZ==0)&&(buff_read_it/RNG_BLOCK_SZ==block_write_it))
        {
            wake_up_interruptible(&read_queue);
            wake_up_interruptible(&poll_queue);
            qrng_ready = true;
        }

        block_write_it = (block_write_it+1)%RNG_BLOCK_CNT;
        buff_ready_cnt++;

        mutex_unlock(&buff_mutex); 
        return copied;
    }else {
        pr_info("QRNG: failed to lock mutex in write_random_bytes_qrng\n");
        return 0;
    }
    return 0;
}

static ssize_t get_random_bytes_qrng(struct iov_iter* iter, bool get_all) {
    size_t ret = 0;

    if(unlikely(!iov_iter_count(iter)))
        return 0;

    while(iov_iter_count(iter)) {
        u8 b;
        ssize_t r = get_random_byte(&b);
        if(r==0) {
            if(get_all) {
                pr_info("QRNG: waiting for qrng_ready to read %d bytes\n", (int)iov_iter_count(iter));
                if( wait_event_interruptible(read_queue, qrng_ready) != 0 )
                    return -ERESTARTSYS;
                pr_info("QRNG: received qrng_ready in get_random_bytes_qrng\n");
                continue; 
            } else {
                return ret;
            }
        }
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
    bool nonblocking = false;

    nonblocking |= (kiocb->ki_flags & (IOCB_NOWAIT | IOCB_NOIO));
    nonblocking |= (kiocb->ki_filp->f_flags & O_NONBLOCK);

    if (!qrng_ready&&nonblocking)
        return -EAGAIN;

    // printk(KERN_INFO "QRNG read iter size: %d\n", (int)iov_iter_count(iter));
    return get_random_bytes_qrng(iter, !nonblocking);
}

static ssize_t qrng_write_iter(struct kiocb*, struct iov_iter* iter) {

    ssize_t ret = 0;
    size_t copied;

    if(unlikely(!iov_iter_count(iter)))
        return 0;

    while(iov_iter_count(iter)>=RNG_BLOCK_SZ&&buff_ready_cnt<RNG_BLOCK_CNT) {
        copied = write_random_bytes_qrng(iter);
        ret += copied;
    }

    return ret ? ret : -EFAULT;
}

static __poll_t qrng_poll(struct file* file, poll_table* wait) {
    __poll_t res = 0;

	poll_wait(file, &poll_queue, wait);
    
    if(buff_ready_cnt<RNG_BLOCK_CNT) res |= EPOLLOUT;  // writing is now possible
    if(qrng_ready) res |= POLLIN|EPOLLRDNORM;     // there is data to read.

    printk(KERN_INFO "QRNG service polled, returned %d\n", res);

    return res;
}


static struct fasync_struct *fasync;
static int qrng_fasync(int fd, struct file *filp, int on) {
    return fasync_helper(fd, filp, on, &fasync);
}

module_init(qrng_init);
module_exit(qrng_exit);

MODULE_LICENSE("GPL");