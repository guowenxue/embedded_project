/********************************************************************************************
 *        File:  common.h
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  A busy head file
 *   ChangeLog:  1, Release initial version on "Tue Jul 12 16:43:18 CST 2011"
 *
 *******************************************************************************************/

#ifndef __COMMON_H
#define __COMMON_H

#include <stdarg.h>
#include <config.h>
#include <asm/types.h>
#include <asm/io.h>
#include <linux/types.h>

#define DEBUG
#ifdef DEBUG
#define dbg_print(format,args...) printf(format, ##args)
#else
#define dbg_print(format,args...) do{} while(0);
#endif

typedef unsigned char uchar;
typedef volatile unsigned long vu_long;
typedef volatile unsigned short vu_short;
typedef volatile unsigned char vu_char;
typedef volatile unsigned int vu_int;

/*Define in board.c*/
inline void delay(unsigned long loops);
void init_led_beep(void);
void turn_led_on(int led);
void turn_led_off(int led);
void beep(int count);

void serial_init(void);
void serial_send_byte(char c);
int serial_is_recv_enable(void);
int serial_recv_byte(void);
void serial_puts(const char *s);
long xmodem_recv(char *buf);

/*Define in printf.c*/
void * memset(void * s,int c,size_t count);
void printf(const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);


#endif
