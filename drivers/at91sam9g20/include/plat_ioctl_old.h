/*********************************************************************************
 *  Copyright(c)  2011, GHL Sysytems Berhad.
 *  All ringhts reserved.
 *
 *     Filename:  plat_old_ioctl.h
 *  Description:  old ioctl() cmd argument definition here, 
 *                compatible with previous driver and app versions
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-05-10
 *            Author: guoqingdong <guoqingdong@ghlsystems.com>
 *       Descrtipion: Initial first version
 *
 *
 ********************************************************************************/
#ifndef __PLAT_OLD_IOCTL_H
#define __PLAT_OLD_IOCTL_H

/*===========================================================================
 **                 ioctl command for GHL netAccess plat 
 **===========================================================================*/
#define GET_DRV_VER_OLD             33
#define SET_DRV_DEBUG_OLD           17

/*===========================================================================
 *                 ioctl command for few ioctl() cmd driver
 *===========================================================================*/
/* RS485 Device ioctl */
#define RS485_DIR_SEND_OLD          0
#define RS485_DIR_RECV_OLD          1

/* DETECT driver  */
#define DETECT_STATUS_OLD           24
#define DETECT_WIFI_ID_OLD          25

/* BEEP driver */
#define BEEP_ENALARM_OLD            15
#define BEEP_DISALARM_OLD           16
#define SET_DEFAULT_BEEP_FREQ_OLD   8001

/* LED driver */
#define LED_OFF_OLD                 0
#define LED_ON_OLD                  1 
#define LED_BLINK_OLD               3    
#define ALL_LED_OFF_OLD             4
#define ALL_LED_ON_OLD              5 
#define ALL_LED_BLINK_OLD           6    

/*===========================================================================
 *                   ioctl command for GPRS driver
 *===========================================================================*/
#define GPRS_POWERON_OLD            1
#define GPRS_POWERDOWN_OLD          0
#define GPRS_POWERMON_OLD           22
#define GPRS_RESET_OLD              18
#define CHK_WORK_SIMSLOT_OLD        5
#define SET_WORK_SIMSLOT_OLD        6
#define GPRS_GET_RING_OLD           7
#define GPRS_CHK_SIMDOOR_OLD        13
#define GPRS_SET_DTR_OLD            19
#define GPRS_SET_RTS_OLD            20 

/*===========================================================================
 *
 *                   ioctl command for EEPROM driver
 *===========================================================================*/
#define LL_POWERON_OLD              0x8080
#define LL_DATALOW_OLD              0x8081
#define LL_STOP_OLD                 0x8082
#define LL_ACKNAK_OLD               0x8083
#define LL_START_OLD                0x8084
#define LL_WRITE_OLD                0x8085
#define LL_READ_OLD                 0x8086
#define LL_POWEROFF_OLD             0x8087

/*===========================================================================
 *                   ioctl command for Modem driver
 *===========================================================================*/
#define MODEM_POWERON_OLD           1
#define MODEM_RESET_OLD             7
#define MODEM_SET_RTS_OLD           0
#define MODEM_SET_DTR_OLD           5
#define MODEM_CHK_RING_OLD          3
#define MODEM_CHK_LINK_OLD          4
#define MODEM_DISABLE_OLD           8   /*Only for N300*/

/*===========================================================================
 *                   ioctl command for SAM driver
 *===========================================================================*/
#define SAM_SEL_VCC_TYPE_OLD        1
#define SAM_CARD_RESET_OLD          0
#define SAM_CARD_STATUS_OLD         3                
#define SAM_POWER_OFF_OLD           4 
#define SAM_SET_PARA_OLD            5
#define SAM_SET_PROT_OLD            6
#define SAM_POWER_UP_OLD            7 
#define SAM_CARD_ACTIVE_OLD         8 
#define SAM_CARD_DEACTIVE_OLD       SAM_POWER_OFF_OLD

#endif                          /* __PLAT_OLD_IOCTL_H */
