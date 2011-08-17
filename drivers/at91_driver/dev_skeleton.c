/*********************************************************************************
 *  Copyright(c)  2011, Guo Wenxue <guowenxue@gmail.com>
 *  All ringhts reserved.
 *
 *     Filename:  dev_skeleton.c
 *  Description:  Linux common charactor device skeleton driver, it's just provider
 *                ioctl() system call. No read(),write(),ll_seek()... support
 *
 *        Usage:
 *                /tmp >: insmod dev_skeleton.ko  dev_major=218 debug=1
 *                /tmp >: dmesg
 *                Char skeleton driver version 1.0.0 initiliazed
 *                /tmp >: cat /proc/devices | grep skeleton    
 *                218 skeleton
 *                /tmp >: mknod -m 777 /dev/skeleton c 218 09
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-08-10
 *            Author: Guo Wenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 ********************************************************************************/

#include "include/plat_driver.h"

#define DRV_AUTHOR                "GuoWenxue<guowenxue@gmail.com>"
#define DRV_DESC                  "Linux Common Charactor Device Skeleton Driver"

/*Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  SKELETON_NAME

#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*dynamic major by default */
#endif

struct cdev *skeleton_cdev = NULL;

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

#define dbg_print(format,args...) if(DISABLE!=debug) \
                                  {printk("[kernel] ");printk(format, ##args);}

static inline void print_help(void)
{
    printk("--------------------------------------------------------------------\n");
    printk("%s support ioctl command:\n", DEV_NAME);
    printk("List all the support ioctl cmd:  %u\n", GET_DRV_HELP);
    printk("Get driver version information:  %u\n", GET_DRV_VER);
    printk("Enable/Disable driver debug output:  %u [1/0]\n", SET_DRV_DEBUG);
    printk("--------------------------------------------------------------------\n\n");
}

static int skeleton_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int skeleton_release(struct inode *inode, struct file *file)
{
    return 0;
}

static int skeleton_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
                          unsigned long arg)
{
    //int   ret_val;
    switch (cmd)
    {
      case SET_DRV_DEBUG:
          dbg_print("%s driver debug now.\n", DISABLE == arg ? "Disable" : "Enable");

          if (0 == arg)
              debug = DISABLE;
          else
              debug = ENABLE;

          break;

      case GET_DRV_VER:
          print_version(DRV_VERSION);
          return DRV_VERSION;
          break;

      case GET_DRV_HELP:
          print_help();
          break;

      default:
          printk("%s driver don't support ioctl command=%d\n", DEV_NAME, cmd);
          return -1;
    }

    return 0;
}

static struct file_operations skeleton_fops = {
    .owner = THIS_MODULE,
    .open = skeleton_open,
    .release = skeleton_release,
    .ioctl = skeleton_ioctl,
};

static void skeleton_cleanup(void)
{
    dev_t devno = MKDEV(dev_major, dev_minor);

    cdev_del(skeleton_cdev);
    unregister_chrdev_region(devno, 1);

    printk("%s driver removed\n", DEV_NAME);
}

static int __init skeleton_init(void)
{
    int result;
    dev_t devno;

    /*Alloc for the device for driver */
    if (0 != dev_major)
    {
        devno = MKDEV(dev_major, dev_minor);
        result = register_chrdev_region(devno, 1, DEV_NAME);
    }
    else
    {
        result = alloc_chrdev_region(&devno, dev_minor, 1, DEV_NAME);
        dev_major = MAJOR(devno);
    }

    /*Alloc for device major failure */
    if (result < 0)
    {
        printk("%s driver can't get major %d\n", DEV_NAME, dev_major);
        return result;
    }

    /*Alloc cdev structure */
    skeleton_cdev = cdev_alloc();;
    if (NULL == skeleton_cdev)
    {
        printk("%s driver can't alloc for skeleton_cdev\n", DEV_NAME);
        goto ERROR;
    }

    /*Initialize cdev structure and register it */
    skeleton_cdev->owner = THIS_MODULE;
    skeleton_cdev->ops = &skeleton_fops;
    result = cdev_add(skeleton_cdev, devno, 1);
    if (0 != result)
    {
        printk("%s driver can't alloc for skeleton_cdev\n", DEV_NAME);
        goto ERROR;
    }

    printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER,
           DRV_REVER_VER);

    return 0;

  ERROR:
    skeleton_cleanup();
    return -ENODEV;

}

module_init(skeleton_init);
module_exit(skeleton_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
