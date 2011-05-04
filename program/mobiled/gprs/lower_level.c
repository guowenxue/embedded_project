/*****************************************************************************
 * Copyright(c)  2009, GHL Sysytems Berhad.
 *
 * Filename:     lower_level.c
 * Version:      1.0
 * Author:       guowenxue
 * Date:         2012-02-23
 * Description:  ioctrl, datachn mgmt
 *
 * Modification History
 *     1.        Version:       1.0
 *               Date:          2012-02-23
 *               Author:        guowenxue
 *               Modification:  creation
 *****************************************************************************/

#include "plat_ioctl.h"
#include "gprs.h"

int ioctl_set_worksim(int iCtrlport, int iSim)
{
    int iRet = 0;

    if (iCtrlport < 0)
        return FAILED;

    iRet = ioctl(iCtrlport, GPRS_SET_WORKSIM, iSim);
    return iRet;
}

/*No need in this function*/
int ioct_get_worksim(int iCtrlport)
{
    int iRet;

    if (iCtrlport < 0)
        return FAILED;

    iRet = ioctl(iCtrlport, GPRS_GET_WORKSIM, 0);
    return iRet;
}

int ioctl_poweron_gprs(int iCtrlport, int iSim)
{
    int iRet = 0;

    if (iCtrlport < 0)
        return FAILED;

    iRet = ioctl(iCtrlport, GPRS_POWERON, iSim);

    return iRet;
}

int ioctl_check_gprs_power(int iCtrlport)
{
    int iRet = 0;

    if (iCtrlport < 0)
        return FAILED;

    iRet = ioctl(iCtrlport, GPRS_POWERMON, 0);
    return iRet;
}

int ioctl_reset_gprs(int iCtrlport, int iSim)
{
    int iRet = 0;

    if (iCtrlport < 0)
        return FAILED;

    ioctl_set_worksim(iCtrlport, iSim);
    iRet = ioctl(iCtrlport, GPRS_RESET, iSim);
    sleep(1);
    return iRet;
}

int ioctl_poweroff_gprs(int iCtrlport)
{
    int iRet = 0;

    if (iCtrlport < 0)
        return FAILED;

    iRet = ioctl(iCtrlport, GPRS_POWERDOWN, 0);
    return iRet;
}

int ioctl_check_simdoor(int iCtrlport, int iSim)
{
    int iRet = OPENED;

    if (iCtrlport < 0)
        return FAILED;

    iRet = ioctl(iCtrlport, GPRS_CHK_SIMDOOR, iSim);
    if (iRet < 0)
        return FAILED;
    else if (0 == iRet)
    {
        return CLOSED;
    }
    else
    {
        return OPENED;
    }
}
