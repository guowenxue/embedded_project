
#ifndef __PLAT_IOCTL_H
#define __PLAT_IOCTL_H

/*===========================================================================
 *   GPRS Module control/data port device definition
 *==========================================================================*/

#define NOTTY                  0
#define TTY                    1

#define ON                     1
#define OFF                    0

#if ( defined PLAT_L200 ||defined PLAT_L100 )
#define BEEP_ON                1
#define BEEP_OFF               0

#define GPRS_CHANNEL0          "/dev/gprs"
#define GPRS_CHANNEL1          "/dev/ttyS1"
#define UC864E_CHANNEL0        "/dev/ttyUC864E0"
#define UC864E_CHANNEL1        "/dev/ttyUC864E2"

/*  PLAT_L200 End */
#elif (defined PLAT_L300 || defined PLAT_L350 || defined PLAT_L200ARM9)

#define BEEP_ON                15
#define BEEP_OFF               16

#define GPRS_CHANNEL0          "/dev/gprs"
#define GPRS_CHANNEL1          "/dev/ttyS2"
/*On L350, only ttyUC864E0 and ttyUC864E2 are available*/
#define UC864E_CHANNEL0        "/dev/ttyUC864E0"
#define UC864E_CHANNEL1        "/dev/ttyUC864E2"
/*  PLAT_L300 End */

#elif (defined PLAT_N300 || defined PLAT_I386)

#define BEEP_ON                15
#define BEEP_OFF               16

#define UC864E_CHANNEL0        "/dev/ttyUSB0"
#define UC864E_CHANNEL1        "/dev/ttyUSB1"
#endif

/*===========================================================================
 *   Ioctl cmd defined for both GTM900B/LC6311/UC864E/GOSPELL GPRS module
 *==========================================================================*/
#define GPRS_POWERON        1   /*Power Up GPRS */
#define GPRS_POWERDOWN      2   /*Power donw GPRS */
#define GPRS_GET_WORKSIM    5   /*Get which SIM slot work now */
#define GPRS_SET_WORKSIM    6   /*Set which SIM slot work now */
#define GPRS_GET_RI         7   /*Get the RI pin status in GPRS module */
#define GPRS_CHK_SIMDOOR    13  /*Check the SIM door close or not */

#define GPRS_SET_DTR        19  /*Set DTR pin status in GPRS module */
#define GPRS_SET_RTS        20  /*Set RTS pin status in GPRS module */

/*Special ioctl cmd for UC864E driver*/
#define GPRS_DRV_DEBUG      17  /*Enable GPRS driver debug */
#define GPRS_RESET          18  /*GPRS module reset */
#define GPRS_POWERMON       22  /*Get current GPRS power status(up/down) */
#define SET_UC864_USB_WORK  23  /*Set UC864E work or the extern USB interface work */

#define PSTN_RESET          7

#endif                          /*Endif GPRS_IOCTL_H */
