/*********************************************************************************
 *  Copyright(c)  2011, GuoWenxue<guowenxue@gmail.com>.
 *  All ringhts reserved.
 *
 *     Filename:  dev_gprs.c
 *  Description:  AT91SAM9260 platform common char device gprs control port driver.
 *
 *    ChangeLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-05-02
 *            Author: guowenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 ********************************************************************************/

#include "dev_gprs.h"

#define DRV_AUTHOR                "GuoWenxue<guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9260 platform GPRS driver"

/*Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_GPRS_NAME

#ifdef BEEP_DRIVER_DEBUG
int debug = ENABLE;
#else
int debug = DISABLE;
#endif

static int dev_major = DEV_BEEP_MAJOR;
static int dev_minor = 0;

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);

static struct class *dev_class;
static struct cdev *dev_cdev;
int    gprs_index;  /*Opened GPRS index in the support GPRS module list*/

#ifdef CONFIG_PROC_FS
static int read_proc_gprs(char *page, char **start, off_t offset, int count, int *eof, void *data)
{
    char *p = page;
    int i = 0;
    if (0 != offset)
    {
        *eof = 1;
        return 0;
    }

    p += sprintf(p, "Current support GPRS module:\n");
    for (i = 0; i < dev_count; i++)
    {
        p += sprintf(p, "\"/dev/%s%d\"  <===>  %s\n", DEV_NAME, i, support_gprs[i].name);
        p += sprintf(p, "Power on period time        : %lu\n", support_gprs[i].poweron_period_time);
        p += sprintf(p, "Power on AT active time     : %lu\n", support_gprs[i].atcmd_active_time);
        p += sprintf(p, "Power off period time       : %lu\n", support_gprs[i].poweroff_period_time);
        p += sprintf(p, "Power oiff AT inactive time : %lu\n", support_gprs[i].atcmd_inactive_time);
        p += sprintf(p, "Ring call period time       : %lu\n", support_gprs[i].ring_call_time);
        p += sprintf(p, "Ring SMS period time        : %lu\n", support_gprs[i].ring_sms_time);
        p += sprintf(p, "\n");
    }

    return (p - page);
}
#endif

static int dev_open(struct inode *inode, struct file *filp)
{
    gprs_index = NUM(inode->i_rdev);

    if(gprs_index >= dev_count)
         return -ENODEV;

    /*Save open() which GPRS module*/
    filp->private_data = &gprs_index;

    dbg_print("Open %s%d <-> \"%s\"\n", DEV_NAME, gprs_index, support_gprs[gprs_index].name);

    return 0;
}

static int dev_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static int dev_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    int    index = *(int *) (filp->private_data);

    dbg_print("ioctl() on %s%d<->%s driver with cmd=%u arg=%lu\n", DEV_NAME, index, support_gprs[index].name, cmd, arg);

    switch (cmd)
    {
      case SET_DRV_DEBUG:
          printk("%s driver debug now.\n", DISABLE == arg ? "Disable" : "Enable");
          if (0 == arg)
              debug = DISABLE;
          else
              debug = ENABLE;
          return debug;

      case GET_DRV_VER:
          print_version(DRV_VERSION);
          return DRV_VERSION;

      default:
          printk("%s driver don't support ioctl command=%u\n", DEV_NAME, cmd);
          return -ENOTTY;
    }

    return 0;
}

static struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .ioctl = dev_ioctl,
};

static void gprs_cleanup(void)
{
    int i;
    dev_t devno;

#ifdef CONFIG_PROC_FS
    remove_proc_entry(DEV_NAME, NULL);
#endif

    /*Remove device file node */
    for (i = 0; i < dev_count; i++)
    {
        devno = MKDEV(dev_major, i);
        device_destroy(dev_class, devno);
    }
    class_destroy(dev_class);

    /*Release charactor device*/
    cdev_del(dev_cdev);

    /*Release device major number*/
    unregister_chrdev_region(MKDEV(dev_major, 0), dev_count);

    printk("%s driver removed\n", DEV_NAME);
}

static int __init gprs_init(void)
{
    int result;
    int i = 0;
    dev_t devno = MKDEV(dev_major, 0);

    /* Regist device major number */
    if (0 != dev_major)
    {
        devno = MKDEV(dev_major, dev_minor);
        result = register_chrdev_region(devno, dev_count, DEV_NAME);
    }
    else
    {
        result = alloc_chrdev_region(&devno, 0, dev_count, DEV_NAME);
        dev_major = MAJOR(devno);
    }

    /* Alloc for device major failure */
    if (result < 0)
    {
        printk("%s driver can't get major %d\n", DEV_NAME, dev_major);
        return result;
    }

    /* Create charactore device*/
    dev_cdev = cdev_alloc();
    if(NULL != dev_cdev)
    {
        dev_cdev->ops = &dev_fops;
        dev_cdev->owner = THIS_MODULE;
        cdev_add(dev_cdev, devno, dev_count);
    }
    else
    {
        printk("ERROR: %s driver cannot alloc cdev.\n", DEV_NAME);  
        result = -1;
        goto ERROR;
    }

    /* Create device file node */
    dev_class = class_create(THIS_MODULE, DEV_NAME);
    if (IS_ERR(dev_class))
    {
        printk("ERROR: %s driver Can't create dev_class.\n", DEV_NAME);
        result = -1;
        goto ERROR;
    }

    for (i=0; i<dev_count; i++)
    {
        devno = MKDEV(dev_major, i);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,24)
        device_create(dev_class, NULL, devno, NULL, "%s%d", DEV_NAME, i);
#else
        device_create(dev_class, NULL, devno, "%s%d", DEV_NAME, i);
#endif
    }

#ifdef CONFIG_PROC_FS
    create_proc_read_entry(DEV_NAME, 0444, NULL, read_proc_gprs, NULL);
#endif

    printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER);

    return 0;

  ERROR:
    gprs_cleanup();
    return result;
}

module_init(gprs_init);
module_exit(gprs_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
