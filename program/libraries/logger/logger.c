/***********************************************************************
 *        File:  logger.c
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  Logger source code file for application to log 
 *   ChangeLog:  1, Release initial version on 2011.05.27
 *
 ***********************************************************************/
#include "logger.h"

/*==========================================================================================
 *   Description:  This function used to inititalize the logger system.
 *
 *  Paramter[IN]:      level: Logger to log file threshold level
 *                  log_path: Log file path
 *                 file_size: Max log file size, unit Kbytes
 *                     flags: log flags as above
 * Paramter[OUT]:  NULL
 *
 *  Return Value:  The log file FD(File Description)
 *=========================================================================================*/
int logger_init(int level, char *log_path, int file_size, int flags)
{
    int        fd = -1;

    return fd;
}

/*==========================================================================================
 *   Description:  This function used to write the log string to the logger file if level
 *                 less than the level set in logger_init() 
 *
 *  Paramter[IN]:        fd: The log file FD(File Description)
 *                    level: Log the logger level
 *                 app_name: The application name or the module name
 *                msg, ... : The log format string
 *
 * Paramter[OUT]:  NULL
 *
 *  Return Value:  0 - success   Else - failure
 *=========================================================================================*/
int logger(int fd, int level, char *app_name, char *msg, ...)
{
    int        retVal = -1;

    return retVal;
}

/*==========================================================================================
 *   Description:  This function used to write the log buffer to the logger file in HEX mode
 *
 *  Paramter[IN]:        fd: The log file FD(File Description)
 *                    level: Log the logger level
 *                 app_name: The application name or the module name
 *                   prompt: The HEX charactor prompt strings
 *                      buf: The HEX charactor data buf
 *                 buf_size: The HEX charactor data buf size
 *
 * Paramter[OUT]:  NULL
 *
 *  Return Value:  0 - success   Else - failure
 *=========================================================================================*/
int logger_hex(int fd, int level, char *app_name, char *prompt, char *buf, int buf_size)
{
    int        retVal = -1;

    return retVal;
}


/*==========================================================================================
 *   Description:  This function used to terminate the logger system.
 *  Paramter[IN]:  fd - The log file FD(File Description)
 * Paramter[OUT]:  NULL
 *
 *  Return Value:  0 - success   Else - failure
 *=========================================================================================*/
int logger_term(int fd)
{
    int        retVal = -1;

    return retVal;
}

