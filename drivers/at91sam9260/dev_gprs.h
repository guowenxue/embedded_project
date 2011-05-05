#ifndef __DEV_GPRS_H
#define __DEV_GPRS_H

#include "include/plat_driver.h"

/*GPRS Module Type*/
enum
{
    GSM_GTM900B = 0,
    GSM_GTM900C,
    GSM_UC864E,
    GSM_SIM5215,
    GSM_SIM900B,
    GSM_G600B,
};

typedef struct __GSM_DEVICE
{
    const char *name;
    int id;

    /*The GPRS turn GPRS_PWR low level period time to turn GPRS on */
    unsigned long poweron_period_time;
    /*When GPRS power on, delay for some time to wait AT command active */
    unsigned long atcmd_active_time;
    /*The GPRS turn GPRS_PWR low level period time to turn GPRS off */
    unsigned long poweroff_period_time;
    /*When GPRS power off, delay for some time to wait AT command inactive */
    unsigned long atcmd_inactive_time;
    /*When Phone call incoming, the RI pin power level wave */
    unsigned long ring_call_time;
    /*When SMS incoming, the RI pin power level wave */
    unsigned long ring_sms_time;
} GSM_DEVICE;

extern GSM_DEVICE support_gprs[]; /*All the support GPRS list*/
extern int dev_count;
extern int debug;

#define dbg_print(format,args...) if(DISABLE!=debug) {printk("[%s] ", DEV_NAME);printk(format, ##args);}

#define RING_NONE                   0x00    /*No Ring incoming */
#define RING_SMS                    0x01    /*Ring incoming */
#define RING_CALL                   0x02    /*A SMS Ring */

#define SIM1                        1
#define SIM2                        2
#define SIM_DEFAULT                 SIM1

#define SIMDOOR_CLOSE               0
#define SIMDOOR_OPEN                1

#endif
