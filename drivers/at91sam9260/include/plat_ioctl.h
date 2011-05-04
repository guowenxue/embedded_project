/*********************************************************************************
 *  Copyright(c)  2011, GuoWenxue<guowenxue@gmail.com>.
 *  All ringhts reserved.
 *
 *     Filename:  plat_ioctl.h
 *  Description:  ioctl() cmd argument definition here
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-05-02
 *            Author: guowenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 *
 ********************************************************************************/

#ifndef __PLAT_IOCTL_H
#define __PLAT_IOCTL_H

/*===========================================================================
 *                 ioctl command for AT91SAM9260 platform  
 *===========================================================================*/

#define AT91_PLATDRV_MAGIC           0x60

/*===========================================================================
 *                 ioctl command for all the drivers 0x01~0x0F
 *===========================================================================*/

/*args is enable or disable*/
#define SET_DRV_DEBUG               _IOW (AT91_PLATDRV_MAGIC, 0x01, unsigned int)
#define GET_DRV_VER                 _IOR (AT91_PLATDRV_MAGIC, 0x02, unsigned int)

/*===========================================================================
 *                 ioctl command for few ioctl() cmd driver 0x10~0x2F
 *===========================================================================*/
/* Restore key */
#define RESTR_KEY_STATUS            _IOR  (AT91_PLATDRV_MAGIC, 0x14, unsigned int)
#define QUERY_PRESS_TIME            _IOR  (AT91_PLATDRV_MAGIC, 0x15, unsigned int)

/* BEEP driver */
#define BEEP_DISALARM               _IO  (AT91_PLATDRV_MAGIC, 0x16)
#define BEEP_ENALARM                _IO  (AT91_PLATDRV_MAGIC, 0x17)
#define SET_DEFAULT_BEEP_FREQ       _IOW (AT91_PLATDRV_MAGIC, 0x18, unsigned int)

/* LED driver */
#define LED_OFF                     _IO   (AT91_PLATDRV_MAGIC, 0x18)
#define LED_ON                      _IO   (AT91_PLATDRV_MAGIC, 0x19)
/*Let LED blink, argument: 0->HW blink  Else Timer blink, arg is the timeout value*/
#define LED_BLINK                   _IOW  (AT91_PLATDRV_MAGIC, 0x1A, unsigned int)

/*===========================================================================
 *                   ioctl command for GPRS driver 0x30~0x4F
 *===========================================================================*/
#define GPRS_POWERDOWN              _IO  (AT91_PLATDRV_MAGIC, 0x30)
#define GPRS_POWERON                _IOW (AT91_PLATDRV_MAGIC, 0x31, unsigned int)
#define GPRS_RESET                  _IOW (AT91_PLATDRV_MAGIC, 0x32, unsigned int)
#define GPRS_POWERMON               _IO  (AT91_PLATDRV_MAGIC, 0x33)
/*Get Which SIM slot work, ioctl(fd, GET_SIM_SLOT, 0)*/
#define CHK_WORK_SIMSLOT            _IO  (AT91_PLATDRV_MAGIC, 0x34)
/*Set Which SIM slot work now*/
#define SET_WORK_SIMSLOT            _IOW (AT91_PLATDRV_MAGIC, 0x35, unsigned int)
/*Check the specify SIM door status*/
#define GPRS_CHK_SIMDOOR            _IOW(AT91_PLATDRV_MAGIC, 0x36, unsigned int)
#define GPRS_SET_DTR                _IOW (AT91_PLATDRV_MAGIC, 0x37, unsigned int)
#define GPRS_GET_RING               _IOR (AT91_PLATDRV_MAGIC, 0x38, unsigned int)
#define SET_PWUP_TIME               _IOW (AT91_PLATDRV_MAGIC, 0x39, unsigned int)
#define SET_PWDOWN_TIME             _IOW (AT91_PLATDRV_MAGIC, 0x3A, unsigned int)
#define SET_RESET_TIME              _IOW (AT91_PLATDRV_MAGIC, 0x3B, unsigned int)

/*===========================================================================
 *                   ioctl command for EEPROM driver 0x50~0x5F
 *===========================================================================*/
#define LL_POWEROFF                 _IO   (AT91_PLATDRV_MAGIC, 0x50)
#define LL_POWERON                  _IO   (AT91_PLATDRV_MAGIC, 0x51)
#define LL_STOP                     _IO   (AT91_PLATDRV_MAGIC, 0x52)
#define LL_START                    _IO   (AT91_PLATDRV_MAGIC, 0x53)
#define LL_READ                     _IOR  (AT91_PLATDRV_MAGIC, 0x54, unsigned char)
#define LL_WRITE                    _IOW  (AT91_PLATDRV_MAGIC, 0x55, unsigned char)
#define LL_DATALOW                  _IOW  (AT91_PLATDRV_MAGIC, 0x56, unsigned char)
#define LL_ACKNAK                   _IOW  (AT91_PLATDRV_MAGIC, 0x57, unsigned char)


#endif                          /* __PLAT_IOCTL_H */
