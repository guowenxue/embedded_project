/*********************************************************************************
 *      Copyright:  (C) 2011 R&D of San Fran Electronics Co., LTD  
 *                  All rights reserved.
 *
 *       Filename:  at91_adc.c
 *    Description:  This file is the Analog-to-digital Converter(ADC) Driver on AT91.
 *                 
 *        Version:  1.0.0(11/17/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "11/17/2011 04:40:39 PM"
 *                 
 ********************************************************************************/

#include "at91_driver.h"

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9XXX  Analog-to-digital Converter(ADC) driver"

/*  Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_ADC_NAME
//#define DEV_MAJOR                 DEV_ADC_MAJOR

#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*   dynamic major by default */ 
#endif

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;

static struct resource at91_adc_resources[] = {
    [0] = {
        .start = AT91SAM9260_BASE_ADC,
        .end   = AT91SAM9260_BASE_ADC + SZ_16K - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = AT91SAM9260_ID_ADC,
        .end   = AT91SAM9260_ID_ADC,
        .flags = IORESOURCE_IRQ,
    },
};

static struct platform_device at91_adc_device = { 
    .name      = "at91_adc",
    .id        = -1,
#if 0
    .dev       = {
        .platform_data = at91_adc_resources,
    },
#endif
    .resource   = at91_adc_resources,
    .num_resources  = ARRAY_SIZE(at91_adc_resources),
};


static int adc_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int adc_release(struct inode *inode, struct file *file)
{
    return 0;
}

static long adc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return 0;
}


static struct file_operations adc_fops = { 
    .owner = THIS_MODULE, 
    .open = adc_open, 
    .release = adc_release, 
    .unlocked_ioctl = adc_ioctl, /*  compatible with kernel version >=2.6.38*/
};

static int at91_adc_probe(struct platform_device *dev)
{

    return 0;
}


static int at91_adc_remove(struct platform_device *dev)
{
    return 0;
}

static struct platform_driver at91_adc_driver = { 
    .probe      = at91_adc_probe, 
    .remove     = at91_adc_remove, 
    .driver     = { 
        .name       = "at91_adc", 
        .owner      = THIS_MODULE, 
    },
};


static int __init at91_adc_init(void)
{
    int       ret = 0;

    ret = platform_device_register(&at91_adc_device);
    if(ret)
    {
        printk(KERN_ERR "%s:%d: Can't register platform device %s: %d\n", __FUNCTION__,__LINE__,DEV_NAME,ret);
        goto fail_reg_plat_dev;
    }
    dbg_print("Regist AT91 platform %s device successfully.\n", DEV_NAME);

    ret = platform_driver_register(&at91_adc_driver);
    if(ret)
    {
        printk(KERN_ERR "%s:%d: Can't register platform driver %s: %d\n", __FUNCTION__,__LINE__,DEV_NAME,ret);
        goto fail_reg_plat_drv;
    }
    dbg_print("Regist AT91 platform %s driver successfully.\n", DEV_NAME);

fail_reg_plat_drv:
    platform_driver_unregister(&at91_adc_driver);

fail_reg_plat_dev:
    return ret;
}

static void at91_adc_exit(void)
{ 
    dbg_print("%s():%d remove %s platform drvier\n", __FUNCTION__,__LINE__,DEV_NAME); 
    platform_driver_unregister(&at91_adc_driver); 

    dbg_print("%s():%d remove %s platform device\n", __FUNCTION__,__LINE__,DEV_NAME); 
    platform_device_unregister(&at91_adc_device);
}


module_init(at91_adc_init);
module_exit(at91_adc_exit);

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:AT91SAM9260_ADC");

