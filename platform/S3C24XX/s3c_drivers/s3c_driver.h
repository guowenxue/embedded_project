/********************************************************************************
 *      Copyright:  (C) GuoWenxue <guowenxue@gmail.com>  
 *                  All rights reserved.
 *
 *       Filename:  s3c_driver.h
 *    Description:  This is the common head file for S3C24XX common driver
 *
 *        Version:  1.0.0(10/29/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "10/29/2011 11:27:50 AM"
 *                 
 ********************************************************************************/

#ifndef __S3C_DRIVER_H
#define __S3C_DRIVER_H

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
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
#include <linux/poll.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/rtc.h>
#include <linux/spinlock.h>
#include <linux/usb.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <linux/syscalls.h>  /* For sys_access*/
#include <linux/platform_device.h>
#include <linux/unistd.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/irq.h>
#include <mach/regs-gpio.h>


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <asm/irq.h>
#else 
#include <asm-arm/irq.h>
#include <asm/arch/gpio.h>
#include <asm/arch/hardware.h>
#endif
#include "plat_ioctl.h"

/* ===========================================================================
 *         S3C24XX device driver common macro definition 
 *===========================================================================*/

#define ENPULLUP                    1
#define DISPULLUP                   0

#define HIGHLEVEL                   1
#define LOWLEVEL                    0

#define INPUT                       1
#define OUTPUT                      0

#define OFF                         0
#define ON                          1

#define ENABLE                      1
#define DISABLE                     0

#define TRUE                        1
#define FALSE                       0

/* ===========================================================================
 *         S3C24XX device driver name and Major number define 
 *===========================================================================*/


#define DEV_LED_NAME                "led"
#define DEV_LED_MAJOR               203

#define DEV_BUTTON_NAME             "button"
#define DEV_BUTTON_MAJOR            "211"


/*  ***** Bit Operate Define *****/
#define SET_BIT(data, i)   ((data) |=  (1 << (i)))    /*   Set the bit "i" in "data" to 1  */
#define CLR_BIT(data, i)   ((data) &= ~(1 << (i)))    /*   Clear the bit "i" in "data" to 0 */
#define NOT_BIT(data, i)   ((data) ^=  (1 << (i)))    /*   Inverse the bit "i" in "data"  */
#define GET_BIT(data, i)   ((data) >> (i) & 1)        /*   Get the value of bit "i"  in "data" */
#define L_SHIFT(data, i)   ((data) << (i))            /*   Shift "data" left for "i" bit  */
#define R_SHIFT(data, i)   ((data) >> (i))            /*   Shift "data" Right for "i" bit  */


/* ===========================================================================
 *         S3C24XX device driver common function definition 
 *===========================================================================*/

#define SLEEP(x)    {DECLARE_WAIT_QUEUE_HEAD (stSleep); if (10 > x) mdelay ((x * 1000)); \
                        else wait_event_interruptible_timeout (stSleep, 0, (x / 10));}

#define VERSION_CODE(a,b,c)       ( ((a)<<16) + ((b)<<8) + (c))
#define DRV_VERSION               VERSION_CODE(DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER)
#define MAJOR_VER(a)              ((a)>>16&0xFF)
#define MINOR_VER(a)              ((a)>>8&0xFF)
#define REVER_VER(a)              ((a)&0xFF)

#define dbg_print(format,args...) if(DISABLE!=debug) \
        {printk("[kernel] ");printk(format, ##args);}    

static inline void print_version(int version)
{
#ifdef __KERNEL__
        printk("%d.%d.%d\n", MAJOR_VER(version), MINOR_VER(version), REVER_VER(version));
#else
        printf("%d.%d.%d\n", MAJOR_VER(version), MINOR_VER(version), REVER_VER(version));
#endif
}


#endif /* __S3C_DRIVER_H */

