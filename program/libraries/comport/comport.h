/***********************************************************************
 *        File:  comport.h
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  Head file for comport lowlevel shared library API.
 *   ChangeLog:  1, Release initial version on 2011.05.27
 *
 ***********************************************************************/

#ifndef  _COMPORT_H
#define  _COMPORT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>            /*for basename() and dirname() */

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/select.h>
#include <getopt.h>
#include <termios.h>

#define DEVNAME_LEN          64
#define dbg_print printf

typedef struct __COM_PORT
{
    unsigned char databit, parity, stopbit, flowctrl, is_connted;
    long baudrate;
    char dev_name[DEVNAME_LEN];
    int fd_com;
    int frag_size;
} COM_PORT;

void init_comport(COM_PORT * comport);
unsigned char comport_open(COM_PORT * comport);
void set_setting(char *set, COM_PORT * comport);
void disp_setting(COM_PORT * comport);
unsigned char comport_recv(COM_PORT * comport, unsigned char *buf, int buf_size,
                           int *recv_bytes, unsigned long timeout);
unsigned char comport_send(COM_PORT * comport, unsigned char *buf, int send_bytes);
void comport_close(COM_PORT * comport);
void comport_term(COM_PORT * comport);
#endif
