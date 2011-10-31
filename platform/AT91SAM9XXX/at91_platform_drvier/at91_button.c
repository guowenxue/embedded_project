/*********************************************************************************
 *      Copyright:  (C) 2011 Guo Wenxue<guowenxue@gmail.com>  
 *                  All rights reserved.
 *
 *       Filename:  at91_button.c
 *    Description:  This is the common button driver runs on AT91SAM9260/SAM9G20.
 *                 
 *        Version:  1.0.0(10/27/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "10/27/2011 11:39:10 AM"
 *                 
 ********************************************************************************/
#include "at91_driver.h"

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9XXX button driver"

/* Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_LED_NAME

#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /* dynamic major by default */
#endif

/*============================ Button Driver Part ===============================*/
static struct cdev dev_cdev;
static struct class *dev_class = NULL;

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);
#define dbg_print(format,args...) if(DISABLE!=debug) \
        {printk("[kernel] ");printk(format, ##args);}    


/*============================ Platform Device part ===============================*/
/* Button hardware informtation structure*/
struct at91_button_info
{
    int                     nIRQ;     /*Button IRQ number*/
    unsigned int            flags;     /*Button IRQ flags */
    unsigned int            gpio;      /*Button GPIO port */
    unsigned int            num;       /*Button nubmer  */
    char                    *name;     /*Button name  */
};

/* The button plaotform device private data structure */
struct at91_button_platform_data
{
    struct at91_button_info *buttons;
    int                     nbuttons;
};

/* Button hardware informtation data*/
static struct at91_button_info  at91_buttons[] = {
    [0] = {
        .num = 1,
        .name = "key1",
        .nIRQ = AT91_PIN_PB20,
        .gpio = AT91_PIN_PB20,
        .flags = AT91_AIC_SRCTYPE_FALLING,
    },
};

/* The button platform device private data */
static struct at91_button_platform_data at91_button_data = {
    .buttons = at91_buttons,
    .nbuttons = ARRAY_SIZE(at91_buttons),
};

static struct platform_device at91_button_device = {
    .name    = "at91_button",
    .id      = 1,
    .dev     = 
    {
        .platform_data = &at91_button_data, 
    },
};


/*===================== Button device driver part ===========================*/

static int button_open(struct inode *inode, struct file *file)
{ 
    return 0;
}


static int button_release(struct inode *inode, struct file *file)
{ 
    return 0;
}

/*compatible with kernel version >=2.6.38*/
static long button_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{ 
    return 0;
}


static struct file_operations button_fops = { 
    .owner = THIS_MODULE,
    .open = button_open, 
    .release = button_release, 
    .unlocked_ioctl = button_ioctl, /*compatible with kernel version >=2.6.38*/ 
};


static int at91_button_probe(struct platform_device *dev)
{
    int    i, ret = 0;
    struct at91_button_platform_data *pdata = dev->dev.platform_data; 

    for(i=0; i<pdata->nbuttons; i++)
    {
         printk("Register device %s\n", pdata->buttons[i].name); 
    }

    return ret;
}


static int at91_button_remove(struct platform_device *dev)
{
    int    i, ret = 0;
    struct at91_button_platform_data *pdata = dev->dev.platform_data; 

    printk("Remove at91_button now.  \n");

    for(i=0; i<pdata->nbuttons; i++)
    {
         printk("Remove device %s\n", pdata->buttons[i].name); 
    }

    return ret;
}


/*===================== Platform Device and driver regist part ===========================*/

static struct platform_driver at91_button_driver = { 
    .probe      = at91_button_probe, 
    .remove     = at91_button_remove, 
    .driver     = { 
        .name       = "at91_button", 
        .owner      = THIS_MODULE, 
    },
};


static int __init at91_button_init(void)
{
   int       ret = 0;

   ret = platform_device_register(&at91_button_device);
   if(ret)
   {
        printk(KERN_ERR "%s: Can't register platform device %d\n", __FUNCTION__, ret); 
        goto fail_reg_plat_dev;
   }
   printk("Regist AT91 button Device successfully.\n");

   ret = platform_driver_register(&at91_button_driver);
   if(ret)
   {
        printk(KERN_ERR "%s: Can't register platform driver %d\n", __FUNCTION__, ret); 
        goto fail_reg_plat_drv;
   }
   printk("Regist AT91 button Driver successfully.\n");

   return 0;

fail_reg_plat_drv:
   platform_driver_unregister(&at91_button_driver);
fail_reg_plat_dev:
   return ret;
}


static void at91_button_exit(void)
{
    printk("Remove AT91 button device and driver.\n");
    platform_driver_unregister(&at91_button_driver);
    platform_device_unregister(&at91_button_device);
}

module_init(at91_button_init);
module_exit(at91_button_exit);

MODULE_AUTHOR("GuoWenxue <guowenxue@gmail.com>");
MODULE_DESCRIPTION("AT91SAM9260 button driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:AT91SAM9260_button");

