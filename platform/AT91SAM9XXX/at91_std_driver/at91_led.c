/*********************************************************************************
 *      Copyright:  (C) 2011 Guo Wenxue<guowenxue@gmail.com>  
 *                  All rights reserved.
 *
 *       Filename:  at91_led.c
 *    Description:  This is the common LED driver runs on AT91SAM9260/SAM9G20.
 *                 
 *        Version:  1.0.0(10/27/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "10/27/2011 11:39:10 AM"
 *                 
 ********************************************************************************/
#include "at91_driver.h"

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9XXX LED driver"

/*  Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_LED_NAME

//#define DEV_MAJOR                 DEV_LED_MAJOR
#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*  dynamic major by default */ 
#endif

#define LED_OFF                   (0<<0)
#define LED_ON                    (1<<0)


struct led_device
{
      struct at91_led_info *leds;
      int                  nleds;
      struct cdev          cdev;
};

/* ============================ Led Driver Part ===============================*/
static struct cdev dev_cdev;
static struct class *dev_class = NULL;

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;
static struct timer_list blink_timer;


/* ============================ Platform Device part ===============================*/
/*  LED hardware informtation structure*/
struct at91_led_info
{
    unsigned char           num;              /* The LED number  */
    unsigned int            gpio;             /* Which GPIO the LED used */  
    unsigned char           active_level;     /* The GPIO pin level(HIGHLEVEL or LOWLEVEL) to turn on or off  */
    unsigned char           status;           /* Current LED status: LED_OFF, LED_ON */
    unsigned char           blink;            /* Blink or not */           
};

/*  The LED plaotform device private data structure */
struct at91_led_platform_data
{
    struct at91_led_info    *leds;
    int                     nleds;
};

/*  LED hardware informtation data*/ 
static struct at91_led_info  at91_leds[] = {
    [0] = {
        .num = 1,
        .gpio = AT91_PIN_PA26,
        .active_level = LOWLEVEL,
        .status = LED_ON,
        .blink = ENABLE,
    },
    [1] = {
        .num = 2,
        .gpio = AT91_PIN_PC3,
        .active_level = LOWLEVEL,
        .status = LED_OFF,
        .blink = DISABLE,
    },
    [2] = {
        .num = 3,
        .gpio = AT91_PIN_PC7,
        .active_level = LOWLEVEL,
        .status = LED_OFF,
        .blink = DISABLE,
    },
    [3] = { 
        .num = 4,
        .gpio = AT91_PIN_PC12,
        .active_level = LOWLEVEL,
        .status = LED_OFF,
        .blink = DISABLE,
    }, 
    [4] = {
        .num = 5,
        .gpio = AT91_PIN_PB21,
        .active_level = LOWLEVEL,
        .status = LED_OFF,
        .blink = DISABLE,
    },
};

static void platform_led_release(struct device * dev)
{
    int i;
    struct at91_led_platform_data *pdata = dev->platform_data; 

    dbg_print("%s():%d\n", __FUNCTION__,__LINE__);

    /* Turn all LED off */
    for(i=0; i<pdata->nleds; i++)
    {
         at91_set_gpio_value(pdata->leds[i].gpio, ~pdata->leds[i].active_level); 
    }
}

/*  The LED platform device private data */
static struct at91_led_platform_data at91_led_data = {
    .leds = at91_leds,
    .nleds = ARRAY_SIZE(at91_leds),
};

static struct platform_device at91_led_device = {
    .name    = "at91_led",
    .id      = 1,
    .dev     = 
    {
        .platform_data = &at91_led_data, 
        .release = platform_led_release,
    },
};



/* ===================== led device driver part ===========================*/

void led_timer_handle(unsigned long data)
{ 
    int  i; 
    struct at91_led_platform_data *pdata = (struct at91_led_platform_data *)data;

    for(i=0; i<pdata->nleds; i++) 
    { 
        if(LED_ON == pdata->leds[i].status)
        {
              at91_set_gpio_value(pdata->leds[i].gpio, pdata->leds[i].active_level); 
        }
        else
        {
              at91_set_gpio_value(pdata->leds[i].gpio, ~pdata->leds[i].active_level); 
        }

        if(ENABLE == pdata->leds[i].blink )  /* LED should blink */
        {
            /* Switch status between 0 and 1 to turn LED ON or off */
            pdata->leds[i].status = pdata->leds[i].status ^ 0x01;  
        }

        mod_timer(&blink_timer, jiffies + 40);
    }
}


static int led_open(struct inode *inode, struct file *file)
{ 
    return 0;
}


static int led_release(struct inode *inode, struct file *file)
{ 
    return 0;
}

/* compatible with kernel version >=2.6.38*/
static long led_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{ 
    return 0;
}


static struct file_operations led_fops = { 
    .owner = THIS_MODULE, 
    .open = led_open, 
    .release = led_release, 
    .unlocked_ioctl = led_ioctl, /* compatible with kernel version >=2.6.38*/
};


static int at91_led_probe(struct platform_device *dev)
{
    struct at91_led_platform_data *pdata = dev->dev.platform_data; 
    int result = 0;
    int i;
    dev_t devno;

    /* Initialize the LED status */
    for(i=0; i<pdata->nleds; i++)
    {
         if(LED_ON == pdata->leds[i].status)
         {
            at91_set_gpio_output(pdata->leds[i].gpio, pdata->leds[i].active_level); 
         }
         else
         {
            at91_set_gpio_output(pdata->leds[i].gpio, ~pdata->leds[i].active_level); 
         }
    }

    /*  Alloc for the device for driver */
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

    /* Initialize cdev structure and register it*/
    cdev_init (&dev_cdev, &led_fops);
    dev_cdev.owner  = THIS_MODULE;

    result = cdev_add (&dev_cdev, devno , 1); 
    if (result) 
    { 
        printk (KERN_NOTICE "error %d add led device", result); 
        goto ERROR; 
    } 
    
    dev_class = class_create(THIS_MODULE, DEV_NAME); 
    if(IS_ERR(dev_class)) 
    { 
        printk("%s driver create class failture\n",DEV_NAME); 
        result =  -ENOMEM; 
        goto ERROR; 
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)     
    device_create(dev_class, NULL, devno, NULL, DEV_NAME);
#else
    device_create (dev_class, NULL, devno, DEV_NAME);
#endif

    /*  Initial the LED blink timer */
    init_timer(&blink_timer);
    blink_timer.function = led_timer_handle;
    blink_timer.data = (unsigned long)pdata;
    add_timer(&blink_timer); 

    printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER); 
    return 0;
               

ERROR: 
    cdev_del(&dev_cdev); 
    unregister_chrdev_region(devno, 1); 
    return result;

}

static int at91_led_remove(struct platform_device *dev)
{
    dev_t devno = MKDEV(dev_major, dev_minor);

    del_timer(&blink_timer);

    device_destroy(dev_class, devno); 
    class_destroy(dev_class); 
    
    cdev_del(&dev_cdev); 
    unregister_chrdev_region(devno, 1); 
    printk("%s driver removed\n", DEV_NAME);

    return 0;
}


static struct platform_driver at91_led_driver = { 
    .probe      = at91_led_probe, 
    .remove     = at91_led_remove, 
    .driver     = { 
        .name       = "at91_led", 
        .owner      = THIS_MODULE, 
    },
};


static int __init at91_led_init(void)
{
   int       ret = 0;

   ret = platform_device_register(&at91_led_device);
   if(ret)
   {
        printk(KERN_ERR "%s:%d: Can't register platform device %d\n", __FUNCTION__,__LINE__, ret); 
        goto fail_reg_plat_dev;
   }
   dbg_print("Regist AT91 LED Platform Device successfully.\n");

   ret = platform_driver_register(&at91_led_driver);
   if(ret)
   {
        printk(KERN_ERR "%s:%d: Can't register platform driver %d\n", __FUNCTION__,__LINE__, ret); 
        goto fail_reg_plat_drv;
   }
   dbg_print("Regist AT91 LED Platform Driver successfully.\n");

   return 0;

fail_reg_plat_drv:
   platform_driver_unregister(&at91_led_driver);
fail_reg_plat_dev:
   return ret;
}


static void at91_led_exit(void)
{
    dbg_print("%s():%d remove LED platform drvier\n", __FUNCTION__,__LINE__);
    platform_driver_unregister(&at91_led_driver);
    dbg_print("%s():%d remove LED platform device\n", __FUNCTION__,__LINE__);
    platform_device_unregister(&at91_led_device);
}

module_init(at91_led_init);
module_exit(at91_led_exit);

MODULE_AUTHOR("GuoWenxue <guowenxue@gmail.com>");
MODULE_DESCRIPTION("AT91SAM9260 LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:AT91SAM9260_led");

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);
