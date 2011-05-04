/*****************************************************************************
 * Modification History
 *     1.        Version:       1.0
 *               Date:          06/11/2009
 *               Author:        guowenxue(guowenxue@gmail.com)
 *               Modification:  Creation
 *****************************************************************************/
#ifndef __COMMON_H
#define __COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h> /*for basename() and dirname() */
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <time.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

/****** Buffsize Define    ****/
#define SIZE_32         32       
#define SIZE_64         64        
#define SIZE_128        128      
#define SIZE_256        256       
#define SIZE_512        512       
#define SIZE_1024       1024       
#define SIZE_2048       2048       
#define SIZE_4096       4096 

/****** Basic Data Type Define *****/
typedef signed char        SCHAR;        /* Signed   8  bit value */
typedef signed char        *PSCHAR;       /* Signed char pointer */
typedef unsigned char      UCHAR;        /* Unsigned 8  bit value */
typedef unsigned char     *PUCHAR;       /* Unsigned 8  bit value */
typedef signed short       SHORT;        /* Signed   16 bit value */
typedef unsigned short     USHORT;       /* Unsigned 16 bit value */
typedef long int           LONG;         /* Signed   32 bit value */
typedef unsigned long int  ULONG;        /* Unsigned 32 bit value */

/****** Bit Operate Define *****/
#define SET_BIT(data, i)   ((data) |=  (1 << (i)))    /* Set the bit "i" in "data" to 1  */
#define CLR_BIT(data, i)   ((data) &= ~(1 << (i)))    /* Clear the bit "i" in "data" to 0 */
#define NOT_BIT(data, i)   ((data) ^=  (1 << (i)))    /* Inverse the bit "i" in "data"  */
#define GET_BIT(data, i)   ((data) >> (i) & 1)        /* Get the value of bit "i"  in "data" */
#define L_SHIFT(data, i)¡¡ ((data) << (i))            /* Shift "data" left for "i" bit  */
#define R_SHIFT(data, i)¡¡ ((data) >> (i))            /* Shift "data" Right for "i" bit  */

#define BIT0            0x01
#define BIT1            0x02
#define BIT2            0x04
#define BIT3            0x08
#define BIT4            0x10
#define BIT5            0x20
#define BIT6            0x40
#define BIT7            0x80

/****** MAX/MIN Macro Define *****/
#define MIN(x, y)       (((x) < (y)) ? (x) : (y))
#define MAX(x, y)       (((x) > (y)) ? (x) : (y))

#define GET_TICKCOUNT(x)   {struct timeval now; gettimeofday(&now, 0); x = now.tv_sec * 1000; \
                                        x += now.tv_usec/1000;}

#define NSLEEP(m)       {struct timespec cSleep; unsigned long ulTmp; cSleep.tv_sec = m / 1000; \
                        if(cSleep.tv_sec == 0) {ulTmp = m * 10000; cSleep.tv_nsec = ulTmp * 100;} \
                        else{cSleep.tv_nsec = 0;}nanosleep(&cSleep, 0);}
#define msleep(m)       usleep(m*1000);

/****** Error Code Define *****/
#define OK        0
#define ERR       -1
#define FAILURE   -1

#define PRGNAME_LEN       15
#define PIDFILE_LEN       2*PRGNAME_LEN+9  /* "/tmp/xx/xx.pid" */


#define PID_ASCII_SIZE  11
#define RUNNING         0x01
#define NOT_RUNNING     0x00

#define MACRO_DEBUG 
#ifdef MACRO_DEBUG
#define dbg_print(format,args...) printf(format, ##args)
#else
#define dbg_print(format,args...) do{} while(0);
#endif

extern unsigned char     g_ucStop; /*Defined in common.c */

/*  Function Declare here  */
extern void usage(char *name);
extern void print_version (char *name);
extern void daemonize (unsigned char ucDebug, int argc, char **argv);
extern void signal_handler (int signo);
extern  int is_myself_running(const char *pid_path);
extern  int record_runpid_file(const char *pid_path);

#define MODE_HEX       0x01
#define MODE_CHAR      0x02
extern void print_buffer(const char *prefix, char *buf, int len, int column, int mode);


#ifdef __cplusplus
}
#endif


#endif /*Undefine __COMMON_H*/

