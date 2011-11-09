/*********************************************************************************
 *      Copyright:  (C) 2011 (C) 2011 China Undergraduate Embedded League(CUEL)
 *                  All rights reserved.
 *
 *       Filename:  CComport.h
 *    Description:  This head file is for the common TTY/Serial port operator library 
 *                   
 *        Version:  1.0.0(10/17/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "10/17/2011 03:33:25 PM"
 *                     
 ********************************************************************************/
#ifndef  _CCOMPORT_H
#define   _CCOMPORT_H

#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <string.h>
#include  <getopt.h>
#include  <fcntl.h>
#include  <errno.h>
#include  <termios.h>
#include  <sys/stat.h>
#include  <sys/wait.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <sys/select.h>

#define BUF_64  64
#define DEVNAME_LEN          64

//#define COM_DEBUG
#ifdef  COM_DEBUG
#define COM_PRINT(format,args...) printf(format, ##args)
#else
#define COM_PRINT(format,args...) do{} while(0);
#endif

//#define msleep(m)               {struct timespec cSleep; cSleep.tv_sec = 0; cSleep.tv_nsec = m * 1000; nanosleep(&cSleep, 0);}

typedef struct __COM_PORT
{
    unsigned char databit, parity, stopbit, flowctrl, is_connted;
    long baudrate;
    char dev_name[DEVNAME_LEN];
    int fd_com;
    int frag_size;
} COM_PORT;

COM_PORT *comport_init(const char *dev_name, int baudrate, const char *settings);
void comport_close(COM_PORT * comport);
int comport_open(COM_PORT * comport);
void comport_term(COM_PORT * comport);
int comport_recv(COM_PORT * comport, unsigned char *buf, int buf_size,
                 int *recv_bytes, unsigned long timeout);
int comport_send(COM_PORT * comport, unsigned char *buf, int send_bytes);

void set_settings(COM_PORT * comport, const char *settings);
void disp_settings(COM_PORT * comport);
void nonblock();
int kbhit();

#endif
