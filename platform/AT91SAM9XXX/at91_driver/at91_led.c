/*********************************************************************************
 *      Copyright:  (C) 2011 R&D of San Fran Electronics Co., LTD  
 *                  All rights reserved.
 *
 *       Filename:  gpio_uart.c
 *    Description:  This a GPIO simulate UART driver for the IrDA communication
 *                 
 *        Version:  1.0.0(10/27/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "10/27/2011 11:39:10 AM"
 *                 
 ********************************************************************************/
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <mach/gpio.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kref.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/bcd.h>
#include <linux/miscdevice.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/rtc.h>
#include <linux/spinlock.h>
#include <linux/usb.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <linux/syscalls.h>  /* For sys_access*/
#include <linux/unistd.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/irq.h>

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9XXX LED driver"

#define AT91_LEDF_ACTLOW           (1<<0)  /*  LED is on when GPIO low */
#define AT91_LEDF_TRISTATE         (1<<1)  /*  tristate to turn off */     


struct at91_gpio_led
{
    char                    *name;
    unsigned int            gpio;   
    unsigned int            flags;
    char                    *def_trigger;
};

struct at91_led_platform_data
{
    struct at91_gpio_led    *leds;
    int                     nleds;
};


static struct at91_gpio_led  at91_leds[] = {
    [0] = {
        .name = "led1",
        .gpio = AT91_PIN_PA26,
        .flags = AT91_LEDF_ACTLOW | AT91_LEDF_TRISTATE,
        .def_trigger = "Sys Run LED",
    },
    [1] = {
        .name = "led2",
        .gpio = AT91_PIN_PC3,
        .flags = AT91_LEDF_ACTLOW | AT91_LEDF_TRISTATE,
        .def_trigger = "RS232_1 TX/RX",
    },
    [2] = {
        .name = "led3",
        .gpio = AT91_PIN_PC7,
        .flags = AT91_LEDF_ACTLOW | AT91_LEDF_TRISTATE,
        .def_trigger = "RS232_2 TX/RX",
    },
    [3] = { 
        .name = "led4", 
        .gpio = AT91_PIN_PC12,
        .flags = AT91_LEDF_ACTLOW | AT91_LEDF_TRISTATE,
        .def_trigger = "GPRS TX/RX",
    }, 
    [4] = {
        .name = "led5",
        .gpio = AT91_PIN_PB21,
        .flags = AT91_LEDF_ACTLOW | AT91_LEDF_TRISTATE,
        .def_trigger = "TBD",
    },
};

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
    },
};


static int at91_led_probe(struct platform_device *dev)
{
    int    i, ret = 0;
    struct at91_led_platform_data *pdata = dev->dev.platform_data; 

    for(i=0; i<pdata->nleds; i++)
    {
         printk("Register device %s\n", pdata->leds[i].name); 
    }

    return ret;
}



static int at91_led_remove(struct platform_device *dev)
{
    int    i, ret = 0;
    struct at91_led_platform_data *pdata = dev->dev.platform_data; 

    printk("Remove at91_led now.  \n");

    for(i=0; i<pdata->nleds; i++)
    {
         printk("Remove device %s\n", pdata->leds[i].name); 
    }

    return ret;
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
        printk(KERN_ERR "%s: Can't register platform device %d\n", __FUNCTION__, ret); 
        goto fail_reg_plat_dev;
   }
   printk("Regist AT91 LED Device successfully.\n");

   ret = platform_driver_register(&at91_led_driver);
   if(ret)
   {
        printk(KERN_ERR "%s: Can't register platform driver %d\n", __FUNCTION__, ret); 
        goto fail_reg_plat_drv;
   }
   printk("Regist AT91 LED Driver successfully.\n");

   return 0;

fail_reg_plat_drv:
   platform_driver_unregister(&at91_led_driver);
fail_reg_plat_dev:
   return ret;
}


static void at91_led_exit(void)
{
    printk("Remove AT91 LED device and driver.\n");
    platform_driver_unregister(&at91_led_driver);
    platform_device_unregister(&at91_led_device);
}

module_init(at91_led_init);
module_exit(at91_led_exit);

MODULE_AUTHOR("GuoWenxue <guowenxue@gmail.com>");
MODULE_DESCRIPTION("AT91SAM9260 LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:AT91SAM9260_led");

