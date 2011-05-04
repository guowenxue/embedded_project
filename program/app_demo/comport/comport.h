#ifndef  _COMPORT_H
#define   _COMPORT_H

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

#define COM_DEBUG 
#ifdef  COM_DEBUG
#define COM_PRINT(format,args...) printf(format, ##args)
#else
#define COM_PRINT(format,args...) do{} while(0);
#endif

typedef struct __COM_PORT
{
       unsigned char       databit, parity, stopbit, flowctrl, is_connted;
       long                baudrate;
       char                dev_name[DEVNAME_LEN];
       int                 fd_com;
       int                 frag_size;
}COM_PORT;

void init_comport(COM_PORT *comport );
unsigned char comport_open (COM_PORT *comport);
void set_setting(char *set, COM_PORT *comport);
void disp_setting(COM_PORT *comport);
void nonblock ();
int kbhit ();
unsigned char comport_recv ( COM_PORT *comport, unsigned char *buf, int buf_size,
		                             int *recv_bytes, unsigned long timeout);
unsigned char comport_send (COM_PORT *comport, unsigned char *buf, int send_bytes);
void comport_close(COM_PORT *comport);
#endif

