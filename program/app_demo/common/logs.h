/*****************************************************************************
 * Modification History
 *     1.        Version:       1.0
 *               Date:          06/11/2009
 *               Author:        guowenxue(guowenxue@gmail.com)
 *               Modification:  Creation
 *****************************************************************************/
#ifndef LOGS_H 
#define LOGS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"


#define _BEBUG                         /* Debug print Switch: ON or NOT  */
#define DBG_BUF_SIZE    512            /* Debug print buffer size*/

#define LOG_DISB        0              /*Disable "Debug" */
#define LOG_CRIT        1              /*Debug Level "Critical" */
#define LOG_ERRO        2              /*Debug Level "ERROR" */
#define LOG_DEBG        3              /*Debug Level "Debug" */
#define LOG_INFO        4              /*Debug Level "Information" */

#define LOG_MAX         LOG_INFO      /*Max Debug Level*/


extern UCHAR        g_ucDebugLev;   /* Defualt debug level will be set in main() */
extern void logMsg(const char cPrior, const char *fmt, ...);
extern void log_demo(void);

#ifdef _BEBUG
/* These MACRO depends on the compiler compatible with C99 standard. 
 * All the L-serials Cross Compiler can works fine. 
 */
#define raw_logs(priority, fmt, ...)  logMsg(priority, fmt, ##__VA_ARGS__)
#define logs(priority, fmt, ...) \
            logMsg(priority, "%s:%d:%s()|| ", __FILE__, __LINE__, __FUNCTION__); \
            logMsg(priority, fmt, ##__VA_ARGS__)
#else
#define raw_logs(priority, fmt, ...)  do{} while(0);
#define logs(priority,fmt, ...) do{} while(0);
#endif /*Endif _BEBUG*/



#ifdef __cplusplus
}
#endif

#endif /* end of #ifndef LOGS_H  */
