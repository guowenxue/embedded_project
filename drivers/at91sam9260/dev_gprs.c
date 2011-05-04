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

#include "include/plat_driver.h"

#define DRV_AUTHOR                "GuoWenxue<guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9260 platform support GSM module driver"

/*Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_GPRS_NAME

#ifndef DEV_MAJOR
#define DEV_MAJOR                 DEV_GPRS_MAJOR    /*dynamic major by default */
#endif

typedef struct __GSM_TYPE
{
    const char *name;
    unsigned long poweron_time;
    unsigned long poweroff_time;
    unsigned long ring_call_time;
    unsigned long ring_sms_time;
} GSM_TYPE;

GSM_TYPE support_gprs[] = {
    [GSM_GTM900B] = {
                     .name = "GTM900B",
                     .poweron_time = 10000,
                     .poweroff_time = 6000,
                     .ring_call_time = 10,
                     .ring_sms_time = 10,
                     },
    [GSM_GTM900C] = {
                     .name = "GTM900C",
                     .poweron_time = 400,
                     .poweroff_time = 8000,
                     .ring_call_time = 10,
                     .ring_sms_time = 10,
                     },
    [GSM_UC864E] = {
                    .name = "UC864E",
                    .poweron_time = 8000,
                    .poweroff_time = 6000,
                    .ring_call_time = 10,
                    .ring_sms_time = 10,
                    },
    [GSM_SIM5215] = {
                     .name = "SIM5215",
                     .poweron_time = 8000,
                     .poweroff_time = 6000,
                     .ring_call_time = 10,
                     .ring_sms_time = 10,
                     },
};

int dev_count = ARRAY_SIZE(support_gprs);   /*Support GPRS device count, ARRAY_SIZE(support_gprs) */

static struct class *at91_device_class;

struct gprs_dev
{
    GSM_TYPE *current_gprs;     /*Current used GPRS module */
    struct cdev cdev;
} *gprs_devices;

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

#define dbg_print(format,args...) if(DISABLE!=debug) {printk(format, ##args);}

#ifdef GPRS_USE_PROC
static int gprs_proc_read(char *page, char **start, off_t offset, int count, int *eof, void *data)
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
        p += sprintf(p, "Power on time     : %lu\n", support_gprs[i].poweron_time);
        p += sprintf(p, "Power off time    : %lu\n", support_gprs[i].poweroff_time);
        p += sprintf(p, "Ring call time    : %lu\n", support_gprs[i].ring_call_time);
        p += sprintf(p, "Ring SMS time     : %lu\n", support_gprs[i].ring_sms_time);
        p += sprintf(p, "\n");
    }

    return (p - page);
}
#endif

static int gprs_open(struct inode *inode, struct file *file)
{
    struct gprs_dev *dev;

    dev = container_of(inode->i_cdev, struct gprs_dev, cdev);
    dbg_print("Open GSM module \"%s\"\n", dev->current_gprs->name);
    return 0;
}

static int gprs_release(struct inode *inode, struct file *file)
{
    struct gprs_dev *dev;
    dev = container_of(inode->i_cdev, struct gprs_dev, cdev);

    dbg_print("Close GSM module \"%s\"\n", dev->current_gprs->name);
    return 0;
}

static int gprs_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
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

      default:
          printk("%s driver don't support ioctl command=%u\n", DEV_NAME, cmd);
          return -ENOTTY;
    }

    return 0;
}

static struct file_operations gprs_fops = {
    .owner = THIS_MODULE,
    .open = gprs_open,
    .release = gprs_release,
    .ioctl = gprs_ioctl,
};

static inline void gprs_setup_cdev(struct gprs_dev *dev, int index)
{
    int err, devno = MKDEV(dev_major, index);

    cdev_init(&dev->cdev, &gprs_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &gprs_fops;
    err = cdev_add(&dev->cdev, devno, 1);

    /* Fail gracefully if need be */
    if (err)
        printk(KERN_NOTICE "Error %d adding scull%d", err, index);
}

static void gprs_cleanup(void)
{
    int i;
    dev_t devno;

#ifdef GPRS_USE_PROC
    remove_proc_entry(FILESYS_GPRS, NULL);
#endif

    for (i = 0; i < dev_count; i++)
    {
        devno = MKDEV(dev_major, i);
        device_destroy(at91_device_class, devno);
        cdev_del(&gprs_devices[i].cdev);
    }

    class_destroy(at91_device_class);

    kfree(gprs_devices);
    unregister_chrdev_region(MKDEV(dev_major, 0), dev_count);

    printk("%s driver removed\n", DEV_NAME);
}

static int __init gprs_init(void)
{
    int result;
    int i;
    dev_t devno = MKDEV(dev_major, 0);

    /*Alloc for the device for driver */
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

    /*Alloc for device major failure */
    if (result < 0)
    {
        printk("%s driver can't get major %d\n", DEV_NAME, dev_major);
        return result;
    }

    gprs_devices = kmalloc(dev_count * sizeof(struct gprs_dev), GFP_KERNEL);
    if (NULL == gprs_devices)
    {
        printk("kmalloc for gprs_devices failure.\n");
        result = -ENOMEM;
        goto ERROR;
    }

    /*Create /sys/class file to support udev */
    at91_device_class = class_create(THIS_MODULE, FILESYS_GPRS);
    if (IS_ERR(at91_device_class))
    {
        printk(KERN_ERR "Err: failed in creating class.\n");
        result = -1;
        goto ERROR;
    }

    memset(gprs_devices, 0, sizeof(struct gprs_dev));
    for (i = 0; i < dev_count; i++)
    {
        gprs_devices[i].current_gprs = &support_gprs[i];
        gprs_setup_cdev(gprs_devices + i, i);
        devno = MKDEV(dev_major, i);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,24)
        device_create(at91_device_class, NULL, devno, NULL, "%s%d", DEV_NAME, i);
#else
        device_create(at91_device_class, NULL, devno, "%s%d", DEV_NAME, i);
#endif
        printk("Initialize \"%s\" as \"%s%d\"\n", gprs_devices[i].current_gprs->name, DEV_NAME, i);
    }

#ifdef GPRS_USE_PROC
    create_proc_read_entry(FILESYS_GPRS, 0444, NULL, gprs_proc_read, NULL);
#endif

    printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER,
           DRV_REVER_VER);

    return 0;

  ERROR:
    unregister_chrdev_region(devno, dev_count);
    gprs_cleanup();
    return result;
}

module_init(gprs_init);
module_exit(gprs_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
