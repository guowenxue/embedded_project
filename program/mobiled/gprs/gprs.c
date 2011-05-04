/*****************************************************************************
 * Copyright(c)  2009, GHL Sysytems Berhad.
 *
 * Filename:     gprs.c
 * Version:      1.0
 * Author:       guowenxue
 * Date:         2012-02-23
 *
 * Modification History
 *     1.        Version:       1.0
 *               Date:          2012-02-23
 *               Author:        guowenxue
 *               Modification:  creation
 *****************************************************************************/

#include "gprs.h"

MODULE_ATTR g_stGsmModuleList[MAX_SUPPORT_GSM] = {
    {GSM_UNKNOW, "Unknow", 0, 0}
    ,
    {GSM_GTM900B, GSM_GTM900B_NAME, NETYPE_GPRS, 8000, 6000}
    ,                           /*Value test on L200 */
    {GSM_GTM900C, GSM_GTM900C_NAME, NETYPE_GPRS, 4000, 8000}
    ,                           /*Value test on L200 */
    {GSM_UC864E, GSM_UC864E_NAME, NETYPE_3G, 8000, 4000}
    ,                           /*UC864E need wait for a moment to release USB */
    {GSM_G600, GSM_G600_NAME, NETYPE_GPRS, 4000, 4000}
    ,                           /*Not test */
    {GSM_LC6311, GSM_LC6311_NAME, NETYPE_3G, 4000, 4000}
    ,                           /*Not test */
};

/*
 *  Description:  The  strcasecmp()  function  shall  compare, while ignoring differences in case, 
 *                skip back space,  the string pointed to by s1 to the string pointed to by s2. 
 *  
 *                For test in China Mobile SIM card, the same SIM card will reigster and get operater
 *                as "China Mobile" on UC864E, but "China  Mobile" (Two backspace here) on GTM900C.
 *                So I write a new function to do the string compare
 *
 *  Return Value: 0 -> Matched   !0 -> Not matched
 *              
 */
int strcasebkspccmp(register const char *s1, register const char *s2)
{
    int r;
    while ('\0' != (*s2))
    {
        /*Skip all space character */
        if (isspace(*s1))
        {
            s1++;
            continue;
        }
        if (isspace(*s2))
        {
            s2++;
            continue;
        }

        r = ((int)(tolower(*((unsigned char *)s1)) - tolower(*((unsigned char *)s2))));
        if (r)
            return r;
        s1++;
        s2++;
    }

    return 0;
}

int gprs_init(GPRS * pstGprs, int iModule, unsigned char ucSim)
{
    int i = 0;
    unsigned char ucTag = 0x00;

    memset(pstGprs, 0, sizeof(GPRS));

    /*Find the GPRS module in the list */
    for (i = 0; i < MAX_SUPPORT_GSM; i++)
    {
        if (iModule == g_stGsmModuleList[i].ucModel)
        {
            pstGprs->pstModule = &(g_stGsmModuleList[i]);
            ucTag = 0x01;
        }
    }

    if (0x01 != ucTag)
    {
        dbg_print("Unsupport GPRS module[%d].\n", iModule);
        return FAILED;
    }
    pstGprs->ucWorkSim = ucSim;

    /*Open GPRS module comport */
    if (NETYPE_GPRS == pstGprs->pstModule->ucNeType)
    {
        //comport_init(&(pstGprs->stComport), GPRS_COMPORT, 115200, 8, 0, 1, 0);
        init_default_comport(&(pstGprs->stComport), GPRS_COMPORT);
    }
    else
    {
        comport_init(&(pstGprs->stComport), GPRS3G_COMPORT, 115200, 8, 0, 1, 0);
    }

    if (0x00 != comport_open(&(pstGprs->stComport)))
    {
        dbg_print("Open GPRS data port \"%s\" failure.\n", GPRS_COMPORT);
        return FAILED;
    }

    /*If it's the GPRS module, then open the control port */
    if (NETYPE_GPRS == pstGprs->pstModule->ucNeType)
    {
        pstGprs->iCtrlport = open(GPRS_CTRLPORT, O_RDWR, 0777);
        if (pstGprs->iCtrlport < 0)
        {
            dbg_print("Open GPRS control port \"%s\" failure.\n", GPRS_CTRLPORT);
            comport_close(&(pstGprs->stComport));
            return FAILED;
        }
    }
    /*If it's 3G module, it will use the GPRS_COMPort as the control port */
    else
    {
        pstGprs->iCtrlport = pstGprs->stComport.fd;
    }

    return SUCCESS;
}

/*
 *  Description:  Detect SIM card door status
 */
int detect_simdoor_status(int iCtrlport, unsigned char ucSim)
{
    return ioctl_check_simdoor(iCtrlport, ucSim);
}

/*
 * Description:  Check the GPRS power status
 * Input value:  ucStatus: wanna check it's power on or power off, it should be POWERON,POWEROFF
 *               iRetries: AT command check retries times
 * Return value: Current GPRS power status, value: POWERON,POWEROFF,UNKNOW
 */
unsigned char check_gprs_power(COM_PORT * pstComport, unsigned char ucStatus, int iRetries)
{
    int i = 0;
    int iRet;
    unsigned char ucPowerStatus;

    for (i = 0; i < iRetries; i++)
    {
        iRet = atcmd_test(pstComport);
        if (SUCCESS == iRet && POWERON == ucStatus)
        {
            dbg_print("GPRS AT command OK, power on already\n");
            ucPowerStatus = POWERON;
            break;
        }
        else if (SUCCESS != iRet && POWEROFF == ucStatus)
        {
            dbg_print("GPRS AT command KO, power off already\n");
            ucPowerStatus = POWEROFF;
            break;
        }

        micro_second_sleep(100);
    }

    return ucPowerStatus;
}

/*
 *   Description: Power up GPRS module and make sure AT command get Reply 
 */
void gprs_power_on(GPRS * pstGprs)
{
    dbg_print("Power on GPRS with SIM%d\n", pstGprs->ucWorkSim);
    ioctl_poweron_gprs(pstGprs->iCtrlport, pstGprs->ucWorkSim);
    micro_second_sleep(pstGprs->pstModule->ulPowerOnTime);
}

/*
 *   Description: Power off GPRS module and make sure AT command can not get reply 
 */
void gprs_power_off(GPRS * pstGprs)
{
    dbg_print("Power off GPRS with SIM%d\n", pstGprs->ucWorkSim);
    ioctl_poweroff_gprs(pstGprs->iCtrlport);
    micro_second_sleep(pstGprs->pstModule->ulPowerOffTime);
}

int check_simcard_validation(COM_PORT * pstComport)
{
    int iRet = SUCCESS;

    iRet = atcmd_test(pstComport);
    if (SUCCESS != iRet)
    {
        dbg_print("AT command test failure.\n");
        goto CleanUp;
    }
    dbg_print("AT command test sucess\n");

#if 1
    iRet = atcmd_ate0(pstComport);
    if (SUCCESS != iRet)
    {
        dbg_print("ATE0 command test failure.\n");
        goto CleanUp;
    }
#endif

    /*AT+CPIN? check SIM insert */
    iRet = check_sim_valid(pstComport, 15);
    if (SUCCESS != iRet)
    {
        dbg_print("AT+CPIN? check SIM card failure.\n");
        goto CleanUp;
    }
    dbg_print("AT+CPIN? test sucess\n");

    /*AT+CSQ check GPRS Signal strength */
    iRet = check_gprs_signal(pstComport, 30);
    if (SUCCESS != iRet)
    {
        dbg_print("AT+CSQ? check GPRS signal failure.\n");
        goto CleanUp;
    }
    dbg_print("AT+CSQ? test sucess\n");

    /*AT+CREG? Check SIM card regist */
    iRet = check_sim_regist(pstComport, 30);
    if (SUCCESS != iRet)
    {
        dbg_print("AT+CREG? Check SIM card regist failure\n");
        goto CleanUp;
    }
    dbg_print("AT+CREG? test sucess\n");

    /*AT+COPS? Check SIM card carrier */
    iRet = get_carrier(pstComport);
    if (SUCCESS != iRet)
    {
        dbg_print("AT+COPS? get SIM card carrier failure\n");
        goto CleanUp;
    }
    dbg_print("AT+COPS? test sucess\n");
  CleanUp:
    return iRet;
}
