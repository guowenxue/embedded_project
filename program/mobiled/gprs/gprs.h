#ifndef __GPRS_H
#define __GPRS_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "plat_ioctl.h"
#include "comport.h"
#include "mobiled.h"
#include "devices.h"

#define MAX_CHN                       5 /*MAX tty channel */

#define CTRLP                         0 /*Control port */
#define COMP1                         1 /*Data port 1  */
#define COMP2                         2 /*Data port 2  */
#define DCOMP                         COMP1 /*Default used comport */

#define DEV_LEN                       64    /*Device Name Length */
#define IP_LEN                        16    /*IP address Length */

#define OPT_LEN                       34    /*Telecom Operators/carrier name length */

#define APN_LEN                       63    /*The Access Point Name length */
#define APN_USR_LEN                   63    /*The length of Username for the APN */
#define APN_PWD_LEN                   63    /*The length of password for the APN */

#define CLOSED                        0x01  /*SIM door closed */
#define OPENED                        0x00  /*SIM door opened */

#define YES                           0x01
#define NO                            0x00

#define UNKNOW                        0xFF  /*Unknow status */

#define CTRL_PORT                     0x01
#define DATA_PORT                     0x02

#define POWEROFF                      0x00  /*GPRS Power Off */
#define POWERON                       0x01  /*GPRS Power On */
#define RESET                         0x02  /*GPRS module reset */

#define VALID                         0x01  /*Valid SIM card */
#define INVALID                       0x00  /*Invalid SIM card */

#define SECONDARY                     0x00
#define PRIMARY                       0x01

#define NOSIM                         0
#define SIM1                          1 /*SIM card 1 */
#define SIM2                          2 /*SIM card 2 */
#define ALLSIM                        3 /*Used to detect all the SIM card */

#define MAX_SLOT                      2
#if (defined PLAT_L100 || defined PLAT_L300)
#define MAX_SIM                       1 /*L100, L300 has only one SIM card */
#else
#define MAX_SIM                       2 /*MAX SIM card configure support */
#endif
/*
 * L100,L300 Only get 1 SIM slot; L200/L200_OPT/L350 get 2 SIM slot
 * L100 only SIM1 available, L300 only SIM2 avialable
 */
#ifdef PLAT_L300
#define DEFAULT_SIM                   SIM2  /*L300 only SIM2 is avialable */
#else
#define DEFAULT_SIM                   SIM1  /*L100 only SIM1 is avialable */
#endif

/*AT+CREG? Return Status*/
#define REG_UNREGIST                  0x00  /*Not registered, ME is not currently search a new operator */
#define REG_HOMEWORK                  0x01  /*Registered, home network */
#define REG_SEARCHING                 0x02  /*Not registered but ME is currently searching a new operator */
#define REG_DENIED                    0x03  /*Registration denied */
#define REG_UNKNOW                    0x04  /*Unknow */
#define REG_ROAMING                   0x05  /*Registered, roaming */

/*The GSM Module hardware model*/
#define GSM_GTM900B_NAME              "GTM900B"
#define GSM_GTM900C_NAME              "GTM900C"
#define GSM_G600_NAME                 "G600"
#define GSM_UC864E_NAME               "UC864E"
#define GSM_LC6311_NAME               "LC6311"

enum __MODULE_MODEL
{
    GSM_UNKNOW = 0x00,
    GSM_GTM900B,
    GSM_GTM900C,
    GSM_G600,
    GSM_UC864E,
    GSM_LC6311,
    MAX_SUPPORT_GSM,
};

#define NETYPE_GPRS                  0x00
#define NETYPE_3G                    0x01

typedef struct __GPRS_MODULE
{
    unsigned char ucModel;      /*GSM Module Model: UC864E,GTM900B,GTM900C,G600 */
    const char *pcModelName;    /*GSM module name */
    unsigned char ucNeType;     /*GPRS support network type, GPRS or 3G */
    unsigned long ulPowerOnTime;    /*After call ioctl() to power up, wait time for AT command Ready */
    unsigned long ulPowerOffTime;   /*After call ioctl() to power off, wait time for AT command not Ready */
} MODULE_ATTR;

/*The APN(Access Point Name) structure*/
typedef struct __APN
{
    unsigned char ucValid;      /*Can used or not */
    char acApn[APN_LEN];
    char acUsername[APN_USR_LEN];
    char acPassword[APN_PWD_LEN];
} APN;

typedef struct __GPRS
{
    MODULE_ATTR *pstModule;
    unsigned char ucWorkSim;    /*Currnt working SIM card number */
    COM_PORT stComport;
    int iCtrlport;
} GPRS;

extern MODULE_ATTR g_stGsmModuleList[MAX_SUPPORT_GSM];  /*Defined in gprs.c */

/*Defined in gprs.c*/
int strcasebkspccmp(register const char *s1, register const char *s2);
int detect_simdoor_status(int istCtrlport, unsigned char ucSim);
int gprs_init(GPRS * pstGprs, int iModule, unsigned char ucSim);
void gprs_power_on(GPRS * pstGprs);
void gprs_power_off(GPRS * pstGprs);
int check_simcard_validation(COM_PORT * pstComport);

/*Defined in lower_level.c*/
int ioctl_set_worksim(int iCtrlport, int iSim);
int ioctl_poweron_gprs(int istCtrlport, int iSim);
int ioctl_reset_gprs(int iCtrlport, int iSim);
int ioctl_poweroff_gprs(int istCtrlport);
int ioctl_check_simdoor(int istCtrlport, int iSim);

/*Defined in atcmd.c*/
int send_atcmd(COM_PORT * pstComport, char *pcATcmd, char *pcExpect,
               char *pcError, unsigned long ulDelay, unsigned long ulGap,
               unsigned long ulTimeout, char *pcReply, int iReply);

int atcmd_test(COM_PORT * pstComport);
int atcmd_ate0(COM_PORT * pstComport);
int check_sim_valid(COM_PORT * pstComport, int iRetries);
int check_sim_regist(COM_PORT * pstComport, int iRetries);
int check_gprs_signal(COM_PORT * pstComport, int iRetries);
int get_carrier(COM_PORT * pstComport);
int atcmd_poweroff(COM_PORT * pstComport, char *pcCmd);
int atcmd_set_apn(COM_PORT * pstComport, APN * apn);

#endif                          /*End of __GPRS_H */
