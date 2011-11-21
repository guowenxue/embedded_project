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

static int at91_adc_remove(struct platform_device *pdev);

static void platform_adc_release(struct device * dev)
{
    return ;
}

static struct platform_device at91_adc_device = { 
    .name      = "at91_adc",
    .id        = -1,
    .resource   = at91_adc_resources,
    .num_resources  = ARRAY_SIZE(at91_adc_resources),
    .dev = {
        .release = platform_adc_release,
    }
};


#define ADC_CHN0        0
#define ADC_CHN1        1
#define ADC_CHN2        2
#define ADC_CHN3        3
#define MAX_ADC_CHN     4
#define ADC_CHN_CNT     2

#define DATA_LEN        10  /* MAX save 10 sample datas */

unsigned int adc_channel_gpio[MAX_ADC_CHN] = {
    AT91_PIN_PC0, AT91_PIN_PC1, AT91_PIN_PC2, AT91_PIN_PC3
};

/* AT91SAM9260 MAX support 4 ADC channels from PC0~PC3  */
struct adc_channel
{
    int                 id;         /* Which port we used, AT91_PIN_PC0~AT91_PIN_PC3 */
    unsigned int        sample_data[DATA_LEN];
    struct timer_list   sample_timer;
    struct cdev         cdev; 
    int                 devno;
};

struct adc_dev
{
    void __iomem        *io;
    struct class        *class;
    int                 chn_cnt;
    struct adc_channel  *channel;
};

static struct adc_dev *adc;

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

/* Malloc for the adc device and channels  */
struct adc_dev * setup_adc_device(void)
{  

    int i,ret = 0;
    struct adc_dev *adc = NULL;
    struct adc_channel *channel = NULL;

    adc = kmalloc(sizeof(struct adc_dev), GFP_KERNEL);
    if (!adc) 
        goto ERR_ADC_MALLOC;

    memset(adc, 0, sizeof(struct adc_dev));

    adc->class = class_create(THIS_MODULE, DEV_NAME);
    if(IS_ERR(adc->class)) 
    { 
        printk("%s driver create class failture\n",DEV_NAME);
        goto ERR_CLASS_CREAT;
    }

    adc->chn_cnt = 2;
    adc->channel = kmalloc(adc->chn_cnt*sizeof(struct adc_channel), GFP_KERNEL);
    if(!adc->channel)
    {
        goto ERR_CHANNEL_MALLOC;
    }

    for(i=0; i<adc->chn_cnt; i++)
    {
        memset(&(adc->channel[i]), 0, sizeof(struct adc_channel));
        channel = &(adc->channel[i]);

        channel->devno = MKDEV(dev_major, dev_minor+i);
        cdev_init(&channel->cdev, &adc_fops);
        channel->cdev.owner = THIS_MODULE; 
        channel->cdev.ops = &adc_fops;

        ret = cdev_add (&channel->cdev, channel->devno, 1);
        if(ret)
        {
            printk(KERN_NOTICE "Error[%d] adding %s%d", ret, DEV_NAME, i);
            goto ERR_DEV_CREATE;
        }
        else
        { 
            device_create(adc->class, NULL, channel->devno, NULL, "%s%d", DEV_NAME, i);
            dbg_print("cdev_add %s%d successfully.\n", DEV_NAME, i);
        }

    }

    adc->channel[0].id = ADC_CHN0; 
    adc->channel[1].id = ADC_CHN3; 
    dbg_print("setup_adc_device() ok.\n");

    return adc;

ERR_DEV_CREATE:
    kfree(adc->channel);

ERR_CHANNEL_MALLOC:
    class_destroy(adc->class);

ERR_CLASS_CREAT:
    kfree(adc);

ERR_ADC_MALLOC:
    return NULL;
}


static int at91_adc_probe(struct platform_device *pdev)
{
    struct resource *res;
    int size;
    int ret = 0;
    dev_t devno;

    /* Alloc the device for driver */ 
    if (0 != dev_major) 
    { 
        devno = MKDEV(dev_major, dev_minor); 
        ret = register_chrdev_region(devno, 1, DEV_NAME); 
    } 
    else 
    { 
        ret = alloc_chrdev_region(&devno, dev_minor, 1, DEV_NAME); 
        dev_major = MAJOR(devno); 
    } 
    
    if (ret < 0) 
    {    
        printk(KERN_WARNING "%s: can't get major %d\n", DEV_NAME, dev_major); 
        return ret;   
    }  

    /* Set up the device information and add the device */
    adc = setup_adc_device();
    if(NULL == adc)
    {
        printk("setup_adc_device() failure: ret=%d\n", ret);
        return ret;
        goto adc_del_dev;
    }

    platform_set_drvdata(pdev, adc);

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if( !res )
    {
        printk("%s[%04d] failed to get %s memory regist.\n", __FILE__, __LINE__, DEV_NAME);
        ret = -ENXIO;
        goto adc_del_channel;
    }

    size = resource_size(res); 
    if( !request_mem_region(res->start, size, pdev->name) )
    {
        printk("%s[%04d] failed to get %s memory region.\n", __FILE__, __LINE__, DEV_NAME);
        ret = -ENOENT;
        goto adc_del_channel;
    }

    adc->io = ioremap(res->start, size);
    if( !adc->io )
    {
        printk("%s[%04d] %s ioremap() of registers failed.\n", __FILE__, __LINE__, DEV_NAME);
        ret = -ENXIO;
        goto adc_release_mem;
    }

    printk("at91_adc driver probe successfully!\n");
    return 0;

adc_release_mem:
    release_mem_region(res->start, size);

adc_del_channel:
    at91_adc_remove(pdev);

adc_del_dev: 
    unregister_chrdev_region(MKDEV(dev_major, dev_minor), 1);

    return ret;
}


static int at91_adc_remove(struct platform_device *pdev)
{
    struct adc_dev *adc = platform_get_drvdata(pdev);
    struct resource *res;
    int size = 0;

    iounmap(adc->io);

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if( !res )
    {
        printk("%s[%04d] failed to get %s memory regist.\n", __FILE__, __LINE__, DEV_NAME);
    }
    else
    { 
        size = resource_size(res); 
        release_mem_region(res->start, size);
    }

    if(NULL == adc)
    {
        goto RET;
    }

    if(NULL == adc->channel) 
    {
        kfree(adc);
        goto RET;
    }

    if(NULL != adc->channel) 
    { 
        int i;
        for(i=0; i<adc->chn_cnt; i++)
        {
            dbg_print("Desdroy %s%d ID=%d.\n", DEV_NAME, i, adc->channel[i].id);
            cdev_del(&(adc->channel[i].cdev));
            device_destroy(adc->class, adc->channel[i].devno);
        } 
    }

    class_destroy(adc->class);
  
    kfree(adc->channel); 
    kfree(adc);

    unregister_chrdev_region(MKDEV(dev_major, dev_minor), 1);
    printk("AT91 %s driver removed\n", DEV_NAME);

RET:
    platform_set_drvdata(pdev, NULL);
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

    return 0;

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

