/*********************************************************************************
 *  Copyright(c)  2011,  GuoWenxue<guowenxue@gmail.com>..
 *  All ringhts reserved.
 *
 *     Filename:  plat_driver.h
 *  Description:  Common head file, and marcro for all the device driver
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-05-02
 *            Author: guowenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 *
 ********************************************************************************/
#ifndef __PLAT_DRIVER_H
#define __PLAT_DRIVER_H

#include "sys_include.h"
#include "plat_ioctl.h"

/*Plat dependency head file*/
#if (defined PLAT_GR01)
#include "plat_gr01.h"
#endif

/*===========================================================================
 *         AT91SAM9260 GR01 device driver name and Major number define 
 *===========================================================================*/

#define DEV_LED_NAME             "led"
#define DEV_BEEP_NAME            "beep"
#define DEV_GPRS_NAME            "gprs"
#define DEV_EEPROM_NAME          "eeprom"
#define SKELETON_NAME            "skeleton"

/*Dynamic alloc for device major number*/
#define USE_DYNAMIC_MAJOR        1

#ifdef USE_DYNAMIC_MAJOR
#define DEV_LED_MAJOR            0
#define DEV_BEEP_MAJOR           0
#define DEV_GPRS_MAJOR           0
#define DEV_EEPROM_MAJOR         0
#define DEV_SKELETON_MAJOR       0

#else /*Use static major number, it maybe confict with other driver in future.*/
#define DEV_LED_MAJOR            203
#define DEV_BEEP_MAJOR           204
#define DEV_GPRS_MAJOR           248
#define DEV_EEPROM_MAJOR         206
#define DEV_SKELETON_MAJOR       218
#endif

/*===========================================================================
 *                  Common Macro define 
 *===========================================================================*/
#define ENABLE                   1
#define DISABLE                  0

#define ENPULLUP                 1
#define DISPULLUP                0

#define HIGHLEVEL                1
#define LOWLEVEL                 0

#define TYPE(dev)                (MINOR(dev) >> 4)
#define NUM(dev)                 (MINOR(dev) & 0xf)

#define SLEEP(x)    {DECLARE_WAIT_QUEUE_HEAD (stSleep); if (10 > x) mdelay ((x * 1000)); \
                    else wait_event_interruptible_timeout (stSleep, 0, (x / 10));}

#define VERSION_CODE(a,b,c)      ( ((a)<<16) + ((b)<<8) + (c))
#define DRV_VERSION              VERSION_CODE(DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER)
#define MAJOR_VER(a)             ((a)>>16&0xFF)
#define MINOR_VER(a)             ((a)>>8&0xFF)
#define REVER_VER(a)             ((a)&0xFF)

/* ***** Bit Operate Define *****/
#define SET_BIT(data, i)   ((data) |=  (1 << (i)))    /*  Set the bit "i" in "data" to 1  */
#define CLR_BIT(data, i)   ((data) &= ~(1 << (i)))    /*  Clear the bit "i" in "data" to 0 */
#define NOT_BIT(data, i)   ((data) ^=  (1 << (i)))    /*  Inverse the bit "i" in "data"  */
#define GET_BIT(data, i)   ((data) >> (i) & 1)        /*  Get the value of bit "i"  in "data" */
#define L_SHIFT(data, i)?? ((data) << (i))            /*  Shift "data" left for "i" bit  */
#define R_SHIFT(data, i)?? ((data) >> (i))            /*  Shift "data" Right for "i" bit  */

static inline void print_version(int version)
{
#ifdef __KERNEL__
    printk("%d.%d.%d\n", MAJOR_VER(version), MINOR_VER(version), REVER_VER(version));
#else
    printf("%d.%d.%d\n", MAJOR_VER(version), MINOR_VER(version), REVER_VER(version));
#endif
}

#endif /*__PLAT_DRIVER_H*/
