
#ifndef GPRS_H
#define GPRS_H

#include "comport/comport.h"
#include "common/common.h"
#include "common/logs.h"

#define SIMNONE              0
#define SIM1                 1
#define SIM2                 2

#define SIMDOOR_CLOSED       1
#define SIMDOOR_OPENED       0

#define GPRS_POWERON         1
#define GPRS_POWERDOWN       2
#define GPRS_CHKSIM          13 /* ioctl cmd to check SIM door close or not */
#define GPRS_CHKRING         7  

#define RING_NONE            0x00
#define RING_SMS             0x01
#define RING_CALL            0x02

#define APN_LEN              64

enum
{
   MODULE_GTM900B = 0x01,
   MODULE_G600,
   MODULE_UC864E,
   MODULE_LC6311
};

enum 
{
   MODE_UNKNOW = 0x00,
   MODE_GPRS,
   MODE_EDGE,
   MODE_3G,
   MODE_HSDPA = MODE_3G
};

typedef struct __CTRL_PORT
{
   char              dev_name[DEVNAME_LEN];
   int               fd;                
}CTRL_PORT;

#define  REG_UNREGIST  0x00
#define  REG_HOMEWORK  0x01
#define  REG_ROAMING   0x05

typedef struct __SIM
{   
   unsigned  char     netype;    /* GPRS Module work mode: MODE_GPRS,MODE_EDGE,MODE_HSDPA */
   unsigned char      insert;   /* AT+CPIN? Check SIM insert or not */
   unsigned char      regist;
   char               carrier[APN_LEN]; /* The GPRS provider, get by AT+COPS? */
   char               APN[APN_LEN];  /* APN name */
   char               user[SIZE_32];    /* PPP dial up username */
   char               password[SIZE_32];    /* PPP dial up password */
}SIM;

#define POWER_ON     0x01
#define POWER_OFF    0x00

typedef struct GPRS
{
   unsigned  char    module;   /* GPRS Module model: MODULE_GTM900B, MODULE_UC864E ...*/ 
   unsigned  char    signal;   /* GPRS Signal Quality */
   unsigned  char    power;    /* GPRS Power ON/OFF */
   unsigned  char    door[3];  /* Two SIM door status:door[1]->SIM1 door[2]->SIM2, door[0]->unused*/

   CTRL_PORT         ctrl_port; 
   COM_PORT          data_port;
   SIM               *work_sim; /*Current working SIM pointer*/
} GPRS_ATTR;

int open_gprs(GPRS_ATTR *gprs);
int term_gprs(GPRS_ATTR *gprs);
int gprs_power_up(GPRS_ATTR *gprs);
int gprs_power_down(GPRS_ATTR *gprs);

int ioctl_check_sim_door(GPRS_ATTR *gprs, int sim);
int ioctl_check_gprs_ring(GPRS_ATTR *gprs);

unsigned char atcmd_check_gprs_power(GPRS_ATTR *gprs);
unsigned char atcmd_check_sim_insert(GPRS_ATTR *gprs);
unsigned char atcmd_check_gprs_signal(GPRS_ATTR *gprs);
unsigned char atcmd_check_gprs_register(GPRS_ATTR *gprs);
unsigned char atcmd_check_gprs_carrier(GPRS_ATTR *gprs);
unsigned char atcmd_set_apn(GPRS_ATTR  *gprs);
unsigned char send_atcmd(COM_PORT *st_comport, char *atCmd, char *expect, char *error, 
              unsigned long delay, unsigned long gap, unsigned long timeout, char *content);

#endif

