/*****************************************************************************
 * Modification History
 *     1.        Version:       1.0
 *               Date:          06/11/2009
 *               Author:        guowenxue(guowenxue@gmail.com)
 *               Modification:  creation
 *****************************************************************************/
#include "logs.h"
#include "common.h"


#define RED         "\33[01;31m"
#define MAGENTA     "\33[35m"
#define BLUE        "\33[34m"
#define NORMAL      ""

/*************** Comments of Global Variable *********************************
 * Description : control the diaplay of debug message 
 * Called by   : void log_msg(), 
 *****************************************************************************/
UCHAR        g_ucDebugLev = LOG_DISB;   /* Defualt debug level set in main() */

/*****************************************************************************
 * FunctionName: log_msg
 * Description : Print Debug information
 * Called by   : When need to print debug information
 * Inputs      : const int cPrior: This debug information print level.
 *               const char *fmt:   The debug information content format
 *               ..., 
 * Outputs     : output the string to STDERR
 * Returns     : void
 *****************************************************************************/
void logMsg(const char cPrior, const char *fmt, ...)
{
    char         cMsgBuf[DBG_BUF_SIZE];
    va_list      argptr;

    const char *color_pfx = "", *color_sfx = "\33[0m";

    /* Only display these debug print, which debug level (defined by cPrior) higher than the 
      * threshold debug level, which defined by  globle variable g_ucDebugLev. 
      */
    if (cPrior>g_ucDebugLev || LOG_DISB==g_ucDebugLev ) 
    {
        return;
    }

    /* Load all the debug print information to the buffer. */
    va_start(argptr, fmt);
    vsnprintf(cMsgBuf, DBG_BUF_SIZE, fmt, argptr);
    va_end(argptr);

    switch(cPrior)
    {
            case LOG_CRIT: 
                color_pfx = RED;
                break;
            case LOG_ERRO:
                color_pfx = MAGENTA;
                break;
            case LOG_DEBG:
                color_pfx = BLUE;
                break;
            case LOG_INFO:
                color_pfx = NORMAL;
                color_sfx = "";
                break;
    }

    fprintf(stderr, "%s%s%s", color_pfx, cMsgBuf, color_sfx);

    return;
}

void log_demo(void)
{ 
   logs(LOG_DEBG, "LOG_DEBG-> Current DBG_LEVEL: %d\n", g_ucDebugLev);
   logs(LOG_INFO, "LOG_INFO-> Current DBG_LEVEL: %d\n", g_ucDebugLev); 
   logs(LOG_ERRO, "LOG_ERRO-> Current DBG_LEVEL: %d\n", g_ucDebugLev); 
   logs(LOG_CRIT, "LOG_CRIT-> Current DBG_LEVEL: %d\n", g_ucDebugLev); 
}

