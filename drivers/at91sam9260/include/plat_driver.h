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
 *                  Common Macro define 
 *===========================================================================*/
#define ENPULLUP                    1
#define DISPULLUP                   0

#define HIGHLEVEL                   1
#define LOWLEVEL                    0

#define INPUT                       1
#define OUTPUT                      0

#define ENABLE                      1
#define DISABLE                     0

/* GPRS driver */
#define RING_NONE                   0x00    /*No Ring incoming */
#define RING_SMS                    0x01    /*Ring incoming */
#define RING_CALL                   0x02    /*A SMS Ring */

#define SIM1                        1
#define SIM2                        2
#define SIM_DEFAULT                 SIM1

#define SIMDOOR_CLOSE               0
#define SIMDOOR_OPEN                1

#define GPIOIN                      0
#define GPIOOUT                     1
#define PWM                         2
#define INTERRUPT                   3

/* LED driver */
#define SYS_RUNLED                  0   /*First LED always is the system running LED */

/*===========================================================================
 *         AT91SAM9260 GR01 device driver name and Major number define 
 *===========================================================================*/

#define DEV_LED_NAME             "led"
#define DEV_LED_MAJOR            203

#define DEV_BEEP_NAME            "beep"
#define DEV_BEEP_MAJOR           204

#define DEV_GPRS_NAME            "gprs"
#define DEV_GPRS_MAJOR           248
#define FILESYS_GPRS             "at91_gprs" /*For /proc filesystem and /sys/class */
#define GPRS_USE_PROC           /*Enable GPRS driver proc file system */

#define DEV_EEPROM_NAME          "eeprom"
#define DEV_EEPROM_MAJOR         206

#define SKELETON_NAME               "skelton"
//#define DEV_MAJOR                 218      /*Use dynamic major number*/

#define FAKE_TTY_DRV_NAME           "faketty"
#define FAKE_TTY_NAME               "faketty"
#define FAKE_TTY_MAJOR              197
#define FAKE_TTY_MINORS             1
#define FAKE_TTY_MINOR_BASE         0

/*GPRS Module Type*/
enum
{
    GSM_GTM900B = 0,
    GSM_GTM900C,
    GSM_UC864E,
    GSM_SIM5215,
    GSM_G600,
    GSM_LC6311,
};

#define SLEEP(x)    {DECLARE_WAIT_QUEUE_HEAD (stSleep); if (10 > x) mdelay ((x * 1000)); \
                    else wait_event_interruptible_timeout (stSleep, 0, (x / 10));}

#define VERSION_CODE(a,b,c)       ( ((a)<<16) + ((b)<<8) + (c))
#define DRV_VERSION               VERSION_CODE(DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER)
#define MAJOR_VER(a)              ((a)>>16&0xFF)
#define MINOR_VER(a)              ((a)>>8&0xFF)
#define REVER_VER(a)              ((a)&0xFF)

static inline void print_version(int version)
{
#ifdef __KERNEL__
    printk("%d.%d.%d\n", MAJOR_VER(version), MINOR_VER(version), REVER_VER(version));
#else
    printf("%d.%d.%d\n", MAJOR_VER(version), MINOR_VER(version), REVER_VER(version));
#endif
}

#endif /*__PLAT_DRIVER_H*/
