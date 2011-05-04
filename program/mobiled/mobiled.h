#ifndef __NETACCESS_QCTOO_H
#define __NETACCESS_QCTOO_H

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
#include <sys/ioctl.h>

#include "version.h"
#include "comport.h"
#include "gprs.h"

#define DISABLE            0x00
#define ENABLE             0x01

#define NO                 0x00
#define YES                0x01

#define SUCCESS            0x00
#define FAILED             0x01

#define OK                 0x00 /*Reuturn OK */
#define KO                 0x01 /*Return failure */

extern unsigned char g_ucDebug;
#define dbg_print(format,args...)  if(0x00!=g_ucDebug)  { printf(format, ##args); }

static inline void print_buffer(const char *str, char *pucBuf, int iDataLen)
{
    if (ENABLE == g_ucDebug)
    {
        int i;
        if (NULL != str)
        {
            printf("%s", str);
        }
        for (i = 0; i < iDataLen; i++)
        {
            printf("0x%02x ", *(pucBuf + i));
        }
        printf("\n");
    }
}

typedef struct __DATE_TIME
{
    int iYear;
    int iMonth;
    int iDay;
    int iHour;
    int iMinute;
    int iSecond;
} DATE_TIME;

static inline void micro_second_sleep(unsigned long ms)
{
    struct timespec cSleep;
    unsigned long ulTmp;
    cSleep.tv_sec = ms / 1000;
    if (cSleep.tv_sec == 0)
    {
        ulTmp = ms * 10000;
        cSleep.tv_nsec = ulTmp * 100;
    }
    else
    {
        cSleep.tv_nsec = 0;
    }

    nanosleep(&cSleep, 0);
}

static inline unsigned long time_now()
{
    struct timeval now;

    gettimeofday(&now, 0);
    return (now.tv_sec * 1000) + (now.tv_usec / 1000);
}

static inline unsigned long time_elapsed(unsigned long start)
{
    unsigned long current = time_now();

    if (current < start)
    {
        return (0xFFFFFFFF - start) + current;
    }

    //printf("time_elapsed: %ld\n", current-start);
    return current - start;
}

static inline void get_current_time(DATE_TIME * pstTime)
{
    time_t now = time(NULL);
    struct tm *tnow = localtime(&now);

    memset(pstTime, 0, sizeof(DATE_TIME));

    pstTime->iYear = 1900 + tnow->tm_year;
    pstTime->iMonth = 1 + tnow->tm_mon;
    pstTime->iDay = tnow->tm_mday;

    pstTime->iHour = tnow->tm_hour;
    pstTime->iMinute = tnow->tm_min;
    pstTime->iSecond = tnow->tm_sec;

    return;
}

#endif
