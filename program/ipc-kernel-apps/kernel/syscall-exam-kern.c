//kernel module: syscall-exam-kern.c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include "syscall-exam.h"

#define MYCHRDEV_MAX_MINOR 4
#define MYCHRDEV_CAPACITY 65536

struct mychrdev_data {
        char buf[MYCHRDEV_CAPACITY];
        unsigned int headptr;
        unsigned int tailptr;
};

struct mychrdev_data * mydata[MYCHRDEV_MAX_MINOR];
static atomic_t mychrdev_use_stats[MYCHRDEV_MAX_MINOR];
static int mychrdev_major;

struct mychrdev_private {
        pid_t user_pid;
        char user_name[TASK_COMM_LEN];
        int minor;
        struct mychrdev_data * data;
#define headptr data->headptr
#define tailptr data->tailptr
#define buffer data->buf
};

ssize_t mychrdev_read(struct file * file, char __user * buf, size_t count, loff_t * ppos)
{
        int len;
        struct mychrdev_private * myprivate  = (struct mychrdev_private *)file->private_data;

        len = (int)(myprivate->tailptr - myprivate->headptr);

        if (*ppos >= len) {
                return 0;
        }

        if (*ppos + count > len) {
                count = len - *ppos;
        }

        if (copy_to_user(buf, myprivate->buffer + myprivate->headptr + *ppos, count)) {
                return -EFAULT;
        }
        *ppos += count;

        return count;
}

ssize_t mychrdev_write(struct file * file, const char __user * buf, size_t count, loff_t * ppos)
{
        int leftlen;
        struct mychrdev_private * myprivate  = (struct mychrdev_private *)file->private_data;

        leftlen = (MYCHRDEV_CAPACITY - myprivate->tailptr);

        if (* ppos >= MYCHRDEV_CAPACITY) {
                return -ENOBUFS;
        }

        if (*ppos + count > leftlen) {
                count = leftlen - *ppos;
        }

        if (copy_from_user(myprivate->buffer + myprivate->headptr + *ppos, buf, count)) {
                return -EFAULT;
        }
        *ppos += count;
        myprivate->tailptr += count;

        return count;;
}

int mychrdev_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long argp)
{
        struct mychrdev_private * myprivate = (struct mychrdev_private *)file->private_data;
        mydev_info_t a;
        struct mychrdev_window window;

        switch(cmd) {
                case MYCHRDEV_IOCTL_GET_INFO:
                        a.user_pid = myprivate->user_pid;
                        memcpy(a.user_name, myprivate->user_name, strlen(myprivate->user_name));
                        a.available_len = MYCHRDEV_CAPACITY - myprivate->tailptr;
                        a.len = myprivate->tailptr - myprivate->headptr;
                        a.offset_in_ppage = __pa(myprivate) & 0x00000fff;
                        if (copy_to_user((void *)argp, (void *)&a, sizeof(a))) {
                                return -EFAULT;
                        }
                        break;
                case MYCHRDEV_IOCTL_SET_TRUNCATE:
                        if (copy_from_user(&window, (void *)argp, sizeof(window))) {
                                return -EFAULT;
                        }

                        if (window.head < myprivate->headptr) {
                                return -EINVAL;
                        }

                        if (window.tail > myprivate->tailptr) {
                                return -EINVAL;
                        }

                        myprivate->headptr = window.head;
                        myprivate->tailptr = window.tail;
                        break;
                default:
                        return -EINVAL;
        }

        return 0;

}

int mychrdev_open(struct inode * inode, struct file * file)
{
        struct mychrdev_private * myprivate = NULL;
        int minor;

        if (current->euid != 0) {
                return -EPERM;
        }

        minor = MINOR(inode->i_rdev);

        if (atomic_read(&mychrdev_use_stats[minor])) {
                return -EBUSY;
        }
        else {
                atomic_inc(&mychrdev_use_stats[minor]);
        }

        myprivate = (struct mychrdev_private *)kmalloc(sizeof(struct mychrdev_private), GFP_KERNEL);
        if (myprivate == NULL) {
                return -ENOMEM;
        }

        myprivate->user_pid = current->pid;
        sprintf(myprivate->user_name, "%s", current->comm);
        myprivate->minor = minor;
        myprivate->data = mydata[minor];
        file->private_data = (void *)myprivate;
        return 0;
}

int mychrdev_mmap(struct file * file, struct vm_area_struct * vma)
{
        unsigned long pfn;
        struct mychrdev_private * myprivate  = (struct mychrdev_private *)file->private_data;

        /* Turn a kernel-virtual address into a physical page frame */
        pfn = __pa(&(mydata[myprivate->minor]->buf)) >> PAGE_SHIFT;

        if (!pfn_valid(pfn))
                return -EIO;

        vma->vm_flags |= VM_RESERVED;
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

        /* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
        if (remap_pfn_range(vma,
                            vma->vm_start,
                            pfn,
                            vma->vm_end - vma->vm_start,
                            vma->vm_page_prot))
                return -EAGAIN;

        return 0;
}

int mychrdev_release(struct inode * inode, struct file * file)
{

        atomic_dec(&mychrdev_use_stats[MINOR(inode->i_rdev)]);
        kfree(((struct mychrdev_private *)(file->private_data))->data);
        kfree(file->private_data);
        return 0;
}

loff_t mychrdev_llseek(struct file * file, loff_t offset, int seek_flags)
{
        struct mychrdev_private * myprivate = (struct mychrdev_private *)file->private_data;
        int len = myprivate->tailptr - myprivate->headptr;

        switch (seek_flags) {
                case 0:
                        if ((offset > len)
                                || (offset < 0)) {
                                return -EINVAL;
                        }
                case 1:
                        if ((offset + file->f_pos < 0)
                                || (offset + file->f_pos > len)) {
                                return -EINVAL;
                        }
                        offset += file->f_pos;
                case 2:
                        if ((offset > 0)
                                || (-offset > len)) {
                                return -EINVAL;
                        }
                        offset += len;
                        break;
                default:
                        return -EINVAL;
        }

        if ((offset >= 0) && (offset <= len)) {
                file->f_pos = offset;
                file->f_version = 0;
                return offset;
        }
        else {
                return -EINVAL;
        }
}

struct file_operations mychrdev_fops = {
        .owner = THIS_MODULE,
        .read = mychrdev_read,
        .write = mychrdev_write,
        .ioctl = mychrdev_ioctl,
        .open = mychrdev_open,
        .llseek = mychrdev_llseek,
        .release = mychrdev_release,
        .mmap = mychrdev_mmap,
};


static int __init mychardev_init(void)
{
        int i;

        for (i=0;i<MYCHRDEV_MAX_MINOR;i++) {
                atomic_set(&mychrdev_use_stats[i], 0);
                mydata[i] = NULL;
                mydata[i] = 
                    (struct mychrdev_data *)kmalloc(sizeof(struct mychrdev_data), GFP_KERNEL);
                if (mydata[i] == NULL) {
                        return -ENOMEM;
                }
                memset(mydata[i], 0, sizeof(struct mychrdev_data));
        }

        mychrdev_major = register_chrdev(0, "mychrdev", &mychrdev_fops);
        if (mychrdev_major <= 0) {
                printk("Fail to register char device mychrdev.\n");
                return -1;
        }
        printk("char device mychrdev is registered, major is %d\n", mychrdev_major);

        return 0;
}

static void __exit mychardev_remove(void)
{
        unregister_chrdev(mychrdev_major, NULL);
}

module_init(mychardev_init);
module_exit(mychardev_remove);
MODULE_LICENSE("GPL");
