/*********************************************************************************
 *  Copyright(c)  2011, Guo Wenxue <guowenxue@gmail.com>
 *  All ringhts reserved.
 *
 *     Filename:  at91_ioctl.h
 *  Description:  ioctl() cmd argument definition here
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-08-10
 *            Author: guowenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 *
 ********************************************************************************/

#ifndef __PLAT_IOCTL_H
#define __PLAT_IOCTL_H

#include <asm/ioctl.h>

/*===========================================================================
 *                Common ioctl command definition 
 *===========================================================================*/

#define PLATDRV_MAGIC           0x60

/*===========================================================================
 *                 ioctl command for all the drivers 0x01~0x0F
 *===========================================================================*/

/*args is enable or disable*/
#define SET_DRV_DEBUG               _IO (PLATDRV_MAGIC, 0x01)
#define GET_DRV_VER                 _IO (PLATDRV_MAGIC, 0x02)

/*===========================================================================
 *                 ioctl command for few ioctl() cmd driver 0x10~0x2F
 *===========================================================================*/

/* ADC driver */
#define ADC_SET_INTERVEL            _IO (PLATDRV_MAGIC, 0x11)

/* LED driver */
#define LED_OFF                     _IO (PLATDRV_MAGIC, 0x18)
#define LED_ON                      _IO (PLATDRV_MAGIC, 0x19)
#define LED_BLINK                   _IO (PLATDRV_MAGIC, 0x1A)



/*===========================================================================
 *                   ioctl command for GPRS driver 0x30~0x4F
 *===========================================================================*/
#define GPRS_POWERDOWN              _IO (PLATDRV_MAGIC, 0x30)
#define GPRS_POWERON                _IO (PLATDRV_MAGIC, 0x31)
#define GPRS_RESET                  _IO (PLATDRV_MAGIC, 0x32)
#define GPRS_POWERMON               _IO (PLATDRV_MAGIC, 0x33)
#define GPRS_CHK_SIMDOOR            _IO (PLATDRV_MAGIC, 0x36)
#define GPRS_SET_DTR                _IO (PLATDRV_MAGIC, 0x37)
#define GPRS_SET_RTS                _IO (PLATDRV_MAGIC, 0x38)
#define GPRS_GET_RING               _IO (PLATDRV_MAGIC, 0x39)
#define SET_PWUP_TIME               _IO (PLATDRV_MAGIC, 0x3A)
#define SET_PWDOWN_TIME             _IO (PLATDRV_MAGIC, 0x3B)
#define SET_RESET_TIME              _IO (PLATDRV_MAGIC, 0x3C)
#define GPRS_CHK_MODEL              _IO (PLATDRV_MAGIC, 0x3E) 

/*===========================================================================
 *
 *                   ioctl command for EEPROM driver 0x50~0x5F
 *===========================================================================*/
#define LL_POWEROFF                 _IO (PLATDRV_MAGIC, 0x50)
#define LL_POWERON                  _IO (PLATDRV_MAGIC, 0x51)
#define LL_STOP                     _IO (PLATDRV_MAGIC, 0x52)
#define LL_START                    _IO (PLATDRV_MAGIC, 0x53)
#define LL_READ                     _IO (PLATDRV_MAGIC, 0x54)
#define LL_WRITE                    _IO (PLATDRV_MAGIC, 0x55)
#define LL_DATALOW                  _IO (PLATDRV_MAGIC, 0x56)
#define LL_ACKNAK                   _IO (PLATDRV_MAGIC, 0x57)

#endif                          /* __PLAT_IOCTL_H */
