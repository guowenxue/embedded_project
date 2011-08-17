#ifndef __DEV_GPRS_H
#define __DEV_GPRS_H

#include "include/plat_driver.h"

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9XXX GPRS/3G module driver"

/*Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_GPRS_NAME
#ifndef DEV_MAJOR
#define DEV_MAJOR                 0  /*dynamic major by default*/
#endif

#define TYPE(dev)                 (MINOR(dev) >> 4)
#define NUM(dev)                  (MINOR(dev) & 0xf)
		
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

extern GSM_DEVICE     support_gprs[];
extern int            dev_count;
extern int            debug;

#define dbg_print(format,args...) if(DISABLE != debug) \
			{printk("[kernel] ");printk(format, ##args);}

/*===========================================================
 **                  GPRS low level API
 *===========================================================*/

void gprs_hw_init(int which);

int detect_gprs_model(void);
int gprs_set_worksim(int sim);
int gprs_get_worksim(void);
int gprs_chk_simdoor(int sim);
void gprs_set_dtr(int which, int level);
void gprs_set_rts(int which, int level);
int gprs_powermon(int which);
void gprs_powerup(int which);
void gprs_reset(int which);
void gprs_powerdown(int which);

#endif  /*End __DEV_GPRS_H */


