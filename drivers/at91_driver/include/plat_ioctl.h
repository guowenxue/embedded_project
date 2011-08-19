/*********************************************************************************
 *  Copyright(c)  2011, Guo Wenxue <guowenxue@gmail.com>
 *  All ringhts reserved.
 *
 *     Filename:  plat_ioctl.h
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

/*=========================================================
 *             Some Macro for SAM driver
 *========================================================*/
#define MAX_ATR_SIZE                55
#define ATR_FILED_NAME_SIZE         8

#define VccType5V                   0x50
#define VccType3V3                  0x33
#define VccType3V                   0x30
#define VccType1V8                  0x18
#define VccShutDown                 0x00
#define BOARD_MCK                   ((18432000 * 97 / 9) / 2)

/* error number */
#define PARITY_RX_ERR               -2
#define PARITY_TX_ERR               -3
#define CARD_ERR                    -4
#define NO_CARD_ERR                 -5
#define ATR_LEN_ERR                 -6
#define ATR_VERIFY_ERR              -7
#define APDU_ERR                    -8
#define TIMEROUT_ERR                -9

#ifndef __KERNEL__
enum __ATRDecInfoIndex
{
    inverse_logic = 0,
    ATRProtocolType,
    num_historical_bytes,
    TA_exist,
    TB_exist,
    TC_exist,
    TD_exist,
    ATR_FI_decimal,
    ATR_DI_decimal,
    Extra_Guard_Time,
    max_ATRDecInfoIndex
} ATRDecInfoIndex;

typedef struct __ATRField
{
        unsigned char FieldBuf[MAX_ATR_SIZE];
        char FieldName[MAX_ATR_SIZE][ATR_FILED_NAME_SIZE];

        /*    the buffer to store card property related info which decode from ATR */
        int ATRDecResult[max_ATRDecInfoIndex];
        unsigned char len;
} ATRField;  
#endif

/*===========================================================================
 *                Common ioctl command definition 
 *===========================================================================*/

#define PLATDRV_MAGIC           0x60

/*===========================================================================
 *                 ioctl command for all the drivers 0x01~0x0F
 *===========================================================================*/

/*args is enable or disable*/
#define SET_DRV_DEBUG               _IOW (PLATDRV_MAGIC, 0x01, unsigned int)
#define GET_DRV_VER                 _IOR (PLATDRV_MAGIC, 0x02, unsigned int)

/*===========================================================================
 *                 ioctl command for few ioctl() cmd driver 0x10~0x2F
 *===========================================================================*/
/* RS485 Device ioctl */
#define RS485_DIR_SEND              _IO   (PLATDRV_MAGIC, 0x10)
#define RS485_DIR_RECV              _IO   (PLATDRV_MAGIC, 0x11)

/* DETECT driver  */
#define DETECT_STATUS               _IO   (PLATDRV_MAGIC, 0x12)
#define DETECT_WIFI_ID              _IO   (PLATDRV_MAGIC, 0x13)

/* Restore key */
#define DETECT_RESTORE_KEY          _IO  (PLATDRV_MAGIC, 0x14)
#define QUERY_PRESS_TIME            _IO  (PLATDRV_MAGIC, 0x15)
#define DETECT_FUNC_KEY             _IO  (PLATDRV_MAGIC, 0x1E)

/* BEEP driver */
#define BEEP_DISALARM               _IO  (PLATDRV_MAGIC, 0x16)
#define BEEP_ENALARM                _IO  (PLATDRV_MAGIC, 0x17)
#define SET_DEFAULT_BEEP_FREQ       _IOW (PLATDRV_MAGIC, 0x18, unsigned int)

/* LED driver */
#define LED_OFF                     _IO   (PLATDRV_MAGIC, 0x18)
#define LED_ON                      _IO   (PLATDRV_MAGIC, 0x19)
#define LED_BLINK                   _IOW  (PLATDRV_MAGIC, 0x1A, unsigned int)
#define ALL_LED_OFF                 _IO   (PLATDRV_MAGIC, 0x1B)
#define ALL_LED_ON                  _IO   (PLATDRV_MAGIC, 0x1C)
#define ALL_LED_BLINK               _IOW  (PLATDRV_MAGIC, 0x1D, unsigned int)
/*===========================================================================
 *                   ioctl command for GPRS driver 0x30~0x4F
 *===========================================================================*/
#define GPRS_POWERDOWN              _IO  (PLATDRV_MAGIC, 0x30)
#define GPRS_POWERON                _IOW (PLATDRV_MAGIC, 0x31, unsigned int)
#define GPRS_RESET                  _IOW (PLATDRV_MAGIC, 0x32, unsigned int)
#define GPRS_POWERMON               _IO  (PLATDRV_MAGIC, 0x33)
/*Get Which SIM slot work, ioctl(fd, GET_SIM_SLOT, 0)*/
#define CHK_WORK_SIMSLOT            _IO  (PLATDRV_MAGIC, 0x34)
/*Set Which SIM slot work now*/
#define SET_WORK_SIMSLOT            _IOW (PLATDRV_MAGIC, 0x35, unsigned int)
/*Check the specify SIM door status*/
#define GPRS_CHK_SIMDOOR            _IOW (PLATDRV_MAGIC, 0x36, unsigned int)
#define GPRS_SET_DTR                _IOW (PLATDRV_MAGIC, 0x37, unsigned int)
#define GPRS_SET_RTS                _IOW (PLATDRV_MAGIC, 0x38, unsigned int)
#define GPRS_GET_RING               _IOR (PLATDRV_MAGIC, 0x39, unsigned int)
#define SET_PWUP_TIME               _IOW (PLATDRV_MAGIC, 0x3A, unsigned int)
#define SET_PWDOWN_TIME             _IOW (PLATDRV_MAGIC, 0x3B, unsigned int)
#define SET_RESET_TIME              _IOW (PLATDRV_MAGIC, 0x3C, unsigned int)
#define SET_GPRS_USB_WORK           _IOW (PLATDRV_MAGIC, 0x3D, unsigned int)  /*L2 ARM7 */
#define GPRS_CHK_MODEL              _IO (PLATDRV_MAGIC, 0x3E)  /*L2 ARM9 new version */

/*===========================================================================
 *
 *                   ioctl command for EEPROM driver 0x50~0x5F
 *===========================================================================*/
#define LL_POWEROFF                 _IO   (PLATDRV_MAGIC, 0x50)
#define LL_POWERON                  _IO   (PLATDRV_MAGIC, 0x51)
#define LL_STOP                     _IO   (PLATDRV_MAGIC, 0x52)
#define LL_START                    _IO   (PLATDRV_MAGIC, 0x53)
#define LL_READ                     _IOR  (PLATDRV_MAGIC, 0x54, unsigned char)
#define LL_WRITE                    _IOW  (PLATDRV_MAGIC, 0x55, unsigned char)
#define LL_DATALOW                  _IOW  (PLATDRV_MAGIC, 0x56, unsigned char)
#define LL_ACKNAK                   _IOW  (PLATDRV_MAGIC, 0x57, unsigned char)

/*===========================================================================
 *                   ioctl command for Modem driver, 0x60~0x6F
 *===========================================================================*/
#define MODEM_POWERON               _IO   (PLATDRV_MAGIC,  0x61)
#define MODEM_RESET                 _IO   (PLATDRV_MAGIC,  0x62)
#define MODEM_SET_RTS               _IOW  (PLATDRV_MAGIC,  0x64, unsigned int)
#define MODEM_GET_DCD               _IOW  (PLATDRV_MAGIC,  0x65, unsigned int)
#define MODEM_GET_CTS               _IOW  (PLATDRV_MAGIC,  0x66, unsigned int)
#define MODEM_SET_DTR               _IOW  (PLATDRV_MAGIC,  0x67, unsigned int)
#define MODEM_CHK_RING              _IO   (PLATDRV_MAGIC,  0x68)
#define MODEM_CHK_LINK              _IO   (PLATDRV_MAGIC,  0x69)
#define MODEM_SET_DIALTIMEOUT       _IOW  (PLATDRV_MAGIC,  0x6A, unsigned int)
#define MODEM_GET_DIALNUMBER        _IOR  (PLATDRV_MAGIC,  0x6B, unsigned int)
#define MODEM_DISABLE               _IOR  (PLATDRV_MAGIC,  0x6C, unsigned int)

/*===========================================================================
 *                   ioctl command for SAM driver, 0x70~0x7F
 *===========================================================================*/
#define SAM_POWER_OFF               _IO   (PLATDRV_MAGIC, 0x70)
#define SAM_POWER_UP                _IO   (PLATDRV_MAGIC, 0x71)
#define SAM_CARD_RESET              _IO   (PLATDRV_MAGIC, 0x72)
#define SAM_CARD_ACTIVE             _IO   (PLATDRV_MAGIC, 0x73)
#define SAM_CARD_DEACTIVE           SAM_POWER_OFF
#define SAM_SEL_VCC_TYPE            _IO   (PLATDRV_MAGIC, 0x74)
#define SAM_CARD_STATUS             _IOR  (PLATDRV_MAGIC, 0x75, ATRField)
#define SAM_SET_PARA                _IO   (PLATDRV_MAGIC, 0x76) /*Not used now */
#define SAM_SET_PROT                _IO   (PLATDRV_MAGIC, 0x77) /*Not used now */
#define SAM_PRESENT_DETECT          _IO   (PLATDRV_MAGIC, 0x78) /*L2 new board support it */

#endif                          /* __PLAT_IOCTL_H */
