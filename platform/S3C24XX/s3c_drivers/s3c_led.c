/*********************************************************************************
 *      Copyright:  (C) 2011 Guo Wenxue<guowenxue@gmail.com>  
 *                  All rights reserved.
 *
 *       Filename:  s3c_led.c
 *    Description:  This is the common LED driver runs on S3C24XX.
 *                 
 *        Version:  1.0.0(10/27/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "10/27/2011 11:39:10 AM"
 *                 
 ********************************************************************************/
#include "s3c_driver.h"

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "S3C24XX LED driver"

/* Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_LED_NAME

//#define DEV_MAJOR                 DEV_LED_MAJOR
#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*  dynamic major by default */ 
#endif

#define TIMER_TIMEOUT             40

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;


/* ============================ Platform Device part ===============================*/
/*  LED hardware informtation structure*/
struct s3c_led_info
{
    unsigned char           num;              /* The LED number  */
    unsigned int            gpio;             /* Which GPIO the LED used */  
    unsigned char           active_level;     /* The GPIO pin level(HIGHLEVEL or LOWLEVEL) to turn on or off  */
    unsigned char           status;           /* Current LED status: OFF/ON */
    unsigned char           blink;            /* Blink or not */           
};

/*  The LED platform device private data structure */
struct s3c_led_platform_data
{
    struct s3c_led_info    *leds;
    int                     nleds;
};


/*  LED hardware informtation data*/ 
static struct s3c_led_info  s3c_leds[] = {
    [0] = {
        .num = 1,
        .gpio = S3C2410_GPB(5),
        .active_level = LOWLEVEL,
        .status = OFF,
        .blink = ENABLE,
    },
    [1] = {
        .num = 2,
        .gpio = S3C2410_GPB(6),
        .active_level = LOWLEVEL,
        .status = OFF,
        .blink = DISABLE,
    },
    [2] = {
        .num = 3,
        .gpio = S3C2410_GPB(8),
        .active_level = LOWLEVEL,
        .status = OFF,
        .blink = DISABLE,
    },
    [3] = { 
        .num = 4,
        .gpio = S3C2410_GPB(10),
        .active_level = LOWLEVEL,
        .status = OFF,
        .blink = DISABLE,
    }, 
};

/*  The LED platform device private data */
static struct s3c_led_platform_data s3c_led_data = {
    .leds = s3c_leds,
    .nleds = ARRAY_SIZE(s3c_leds),
};

struct led_device
{
    struct s3c_led_platform_data    *data;
    struct cdev                     cdev;
    struct class                    *dev_class;
    struct timer_list               blink_timer;
} led_device;

static void platform_led_release(struct device * dev)
{
    int i;
    struct s3c_led_platform_data *pdata = dev->platform_data; 

    dbg_print("%s():%d\n", __FUNCTION__,__LINE__);

    /* Turn all LED off */
    for(i=0; i<pdata->nleds; i++)
    {
         s3c2410_gpio_setpin(pdata->leds[i].gpio, ~pdata->leds[i].active_level); 
    }
}

static struct platform_device s3c_led_device = {
    .name    = "s3c_led",
    .id      = 1,
    .dev     = 
    {
        .platform_data = &s3c_led_data, 
        .release = platform_led_release,
    },
};



/* ===================== led device driver part ===========================*/

void led_timer_handler(unsigned long data)
{ 
    int  i; 
    struct s3c_led_platform_data *pdata = (struct s3c_led_platform_data *)data;

    for(i=0; i<pdata->nleds; i++) 
    { 
        if(ON == pdata->leds[i].status)
        {
              s3c2410_gpio_setpin(pdata->leds[i].gpio, pdata->leds[i].active_level); 
        }
        else
        {
              s3c2410_gpio_setpin(pdata->leds[i].gpio, ~pdata->leds[i].active_level); 
        }

        if(ENABLE == pdata->leds[i].blink )  /* LED should blink */
        {
            /* Switch status between 0 and 1 to turn LED ON or off */
            pdata->leds[i].status = pdata->leds[i].status ^ 0x01;  
        }

        mod_timer(&(led_device.blink_timer), jiffies + TIMER_TIMEOUT);
    }
}


static int led_open(struct inode *inode, struct file *file)
{ 
    struct led_device *pdev ;
    struct s3c_led_platform_data *pdata;

    pdev = container_of(inode->i_cdev,struct led_device, cdev);
    pdata = pdev->data;

    file->private_data = pdata;

    return 0;
}


static int led_release(struct inode *inode, struct file *file)
{ 
    return 0;
}

static void print_led_help(void)
{
    printk("Follow is the ioctl() command for LED driver:\n");
    printk("Enable Driver debug command: %u\n", SET_DRV_DEBUG);
    printk("Get Driver verion  command : %u\n", GET_DRV_VER);
    printk("Turn LED on command        : %u\n", LED_ON);
    printk("Turn LED off command       : %u\n", LED_OFF);
    printk("Turn LED blink command     : %u\n", LED_BLINK);
}

/* compatible with kernel version >=2.6.38*/
static long led_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{ 
    struct s3c_led_platform_data *pdata = file->private_data;

    switch (cmd)
    {
        case SET_DRV_DEBUG:
            dbg_print("%s driver debug now.\n", DISABLE == arg ? "Disable" : "Enable");
            debug = (0==arg) ? DISABLE : ENABLE;
            break;
        case GET_DRV_VER:
            print_version(DRV_VERSION);
            return DRV_VERSION;

        case LED_OFF:
            if(pdata->nleds <= arg)
            {
               printk("LED%ld doesn't exist\n", arg);  
               return -ENOTTY;
            }
            pdata->leds[arg].status = OFF;
            pdata->leds[arg].blink = DISABLE;
            break;

        case LED_ON:
            if(pdata->nleds <= arg)
            {
               printk("LED%ld doesn't exist\n", arg);  
               return -ENOTTY;
            }
            pdata->leds[arg].status = ON;
            pdata->leds[arg].blink = DISABLE;
            break;

        case LED_BLINK:
            if(pdata->nleds <= arg)
            {
               printk("LED%ld doesn't exist\n", arg);  
               return -ENOTTY;
            }
            pdata->leds[arg].blink = ENABLE;
            pdata->leds[arg].status = ON;
            break;

        default: 
            dbg_print("%s driver don't support ioctl command=%d\n", DEV_NAME, cmd); 
            print_led_help();
            return -EINVAL;

    }
    return 0;
}


static struct file_operations led_fops = { 
    .owner = THIS_MODULE, 
    .open = led_open, 
    .release = led_release, 
    .unlocked_ioctl = led_ioctl, /* compatible with kernel version >=2.6.38*/
};


static int s3c_led_probe(struct platform_device *dev)
{
    struct s3c_led_platform_data *pdata = dev->dev.platform_data; 
    int result = 0;
    int i;
    dev_t devno;

    /* Initialize the LED status */
    for(i=0; i<pdata->nleds; i++)
    {
         s3c2410_gpio_cfgpin(pdata->leds[i].gpio, S3C2410_GPIO_OUTPUT);
         if(ON == pdata->leds[i].status)
         {
            s3c2410_gpio_setpin(pdata->leds[i].gpio, pdata->leds[i].active_level); 
         }
         else
         {
            s3c2410_gpio_setpin(pdata->leds[i].gpio, ~pdata->leds[i].active_level); 
         }
    }

    /*  Alloc the device for driver */
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

    /* Alloc for device major failure */ 
    if (result < 0) 
    { 
        printk("%s driver can't get major %d\n", DEV_NAME, dev_major); 
        return result; 
    }

    /* Initialize button structure and register cdev*/
    memset(&led_device, 0, sizeof(led_device));
    led_device.data = dev->dev.platform_data;
    cdev_init (&(led_device.cdev), &led_fops);
    led_device.cdev.owner  = THIS_MODULE;

    result = cdev_add (&(led_device.cdev), devno , 1); 
    if (result) 
    { 
        printk (KERN_NOTICE "error %d add %s device", result, DEV_NAME); 
        goto ERROR; 
    } 
    
    led_device.dev_class = class_create(THIS_MODULE, DEV_NAME); 
    if(IS_ERR(led_device.dev_class)) 
    { 
        printk("%s driver create class failture\n",DEV_NAME); 
        result =  -ENOMEM; 
        goto ERROR; 
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)     
    device_create(led_device.dev_class, NULL, devno, NULL, DEV_NAME);
#else
    device_create (led_device.dev_class, NULL, devno, DEV_NAME);
#endif

    /*  Initial the LED blink timer */
    init_timer(&(led_device.blink_timer));
    led_device.blink_timer.function = led_timer_handler;
    led_device.blink_timer.data = (unsigned long)pdata;
    led_device.blink_timer.expires  = jiffies + TIMER_TIMEOUT;
    add_timer(&(led_device.blink_timer)); 

    printk("S3C %s driver version %d.%d.%d initiliazed.\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER); 

    return 0;
               

ERROR: 
    printk("S3C %s driver version %d.%d.%d install failure.\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER); 
    cdev_del(&(led_device.cdev)); 

    unregister_chrdev_region(devno, 1); 
    return result;

}

static int s3c_led_remove(struct platform_device *dev)
{
    dev_t devno = MKDEV(dev_major, dev_minor);

    del_timer(&(led_device.blink_timer));

    cdev_del(&(led_device.cdev)); 
    device_destroy(led_device.dev_class, devno); 
    class_destroy(led_device.dev_class); 
    
    unregister_chrdev_region(devno, 1); 
    printk("S3C %s driver removed\n", DEV_NAME);

    return 0;
}


static struct platform_driver s3c_led_driver = { 
    .probe      = s3c_led_probe, 
    .remove     = s3c_led_remove, 
    .driver     = { 
        .name       = "s3c_led", 
        .owner      = THIS_MODULE, 
    },
};


static int __init s3c_led_init(void)
{
   int       ret = 0;

   ret = platform_device_register(&s3c_led_device);
   if(ret)
   {
        printk(KERN_ERR "%s:%d: Can't register platform device %d\n", __FUNCTION__,__LINE__, ret); 
        goto fail_reg_plat_dev;
   }
   dbg_print("Regist S3C LED Platform Device successfully.\n");

   ret = platform_driver_register(&s3c_led_driver);
   if(ret)
   {
        printk(KERN_ERR "%s:%d: Can't register platform driver %d\n", __FUNCTION__,__LINE__, ret); 
        goto fail_reg_plat_drv;
   }
   dbg_print("Regist S3C LED Platform Driver successfully.\n");

   return 0;

fail_reg_plat_drv:
   platform_driver_unregister(&s3c_led_driver);
fail_reg_plat_dev:
   return ret;
}


static void s3c_led_exit(void)
{
    dbg_print("%s():%d remove LED platform drvier\n", __FUNCTION__,__LINE__);
    platform_driver_unregister(&s3c_led_driver);
    dbg_print("%s():%d remove LED platform device\n", __FUNCTION__,__LINE__);
    platform_device_unregister(&s3c_led_device);
}

module_init(s3c_led_init);
module_exit(s3c_led_exit);

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:S3C24XX_led");


