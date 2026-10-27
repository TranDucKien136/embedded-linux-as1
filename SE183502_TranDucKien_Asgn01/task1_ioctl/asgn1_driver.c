#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include "asgn1_ioctl.h"

#define AS01_DEVICE_NAME "asgn1"
#define AS01_BUFFER_SIZE 1024
#define AS01_MAJOR_NUMBER 244
#define AS01_PREFIX "[KERNEL] "
#define AS01_VERSION "asgn1_driver v1.0-SE183502"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tran Duc Kien - SE183502");
MODULE_DESCRIPTION("Char device driver for assignment 1");

static struct asgn1_driver {
    char buffer[AS01_BUFFER_SIZE];
    int buffer_len;
    int mode;
    int open_count;
    int write_count;
    int read_count;
    int last_write_size;
    struct mutex lock;
} asgn1_drv = { .lock = __MUTEX_INITIALIZER(asgn1_drv.lock),
                .mode = AS01_MODE_ECHO, };

/**
 * asgn1_open - dinh nghia ham open cho device
 * @inode: con tro den struct inode cua device
 * @file: con tro den struct file cua device
 * kernel se goi ham nay khi nguoi dung mo device
 * dem va in ra log khi device duoc mo
 */
static int asgn1_open(struct inode *inode, struct file *file) {
    int count;
    mutex_lock(&asgn1_drv.lock);
    asgn1_drv.open_count++;
    count = asgn1_drv.open_count;
    mutex_unlock(&asgn1_drv.lock);
    pr_info(AS01_DEVICE_NAME ": device opened (open_count = %d)\n", count);
    return 0;
}

/**
 * asgn1_release - dinh nghia ham release cho device
 * @inode: con tro den struct inode cua device
 * @file: con tro den struct file cua device
 * kernel se goi ham nay khi nguoi dung dong device
 * in ra log khi device bi dong
 */
static int asgn1_release(struct inode *inode, struct file *file) {
    pr_info(AS01_DEVICE_NAME ": device closed\n");
    return 0;
}

/**
 * asgn1_read - dinh nghia ham read cho device
 * @file: con tro den struct file cua device
 * @ubuf: con tro den buffer nguoi dung de doc du lieu tu device
 * @count: so byte nguoi dung muon doc tu device
 * @ppos: con tro den vi tri doc hien tai trong buffer cua device
 * kernel se goi ham nay khi nguoi dung doc du lieu tu device
 * tra ve so byte da doc thanh cong, hoac -EFAULT neu co loi xay ra
 */
static ssize_t asgn1_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) {
    size_t nbytes;
    mutex_lock(&asgn1_drv.lock);
    if (*ppos >= asgn1_drv.buffer_len) {
        mutex_unlock(&asgn1_drv.lock);
        return 0; // EOF
    }
    nbytes = min_t(size_t, count, (asgn1_drv.buffer_len - *ppos));
    if (copy_to_user(ubuf, asgn1_drv.buffer + *ppos, nbytes)) {
        mutex_unlock(&asgn1_drv.lock);
        return -EFAULT;
    }
    asgn1_drv.read_count++;
    *ppos += nbytes;
    pr_info(AS01_DEVICE_NAME ": read %zu bytes from device (read_count = %d)\n", nbytes, asgn1_drv.read_count);
    mutex_unlock(&asgn1_drv.lock);
    return nbytes;
}
