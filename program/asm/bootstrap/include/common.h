#ifndef __COMMON_H
#define __COMMON_H

#include <stdarg.h>
#include <board.h>

typedef int  ptrdiff_t;
typedef unsigned int  size_t;

typedef unsigned char       uchar;
typedef volatile unsigned long  vu_long;
typedef volatile unsigned short vu_short;
typedef volatile unsigned char  vu_char;

typedef unsigned char       unchar;
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef unsigned long       ulong;

typedef __signed__ char __s8;
typedef __signed__ short __s16;
typedef __signed__ int __s32;

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int __u32;

typedef     __u8        uint8_t;
typedef     __u16       uint16_t;
typedef     __u32       uint32_t;

typedef unsigned long phys_addr_t;
typedef unsigned long phys_size_t;  /*For vsprintf()*/

/*Define in board.c*/
inline void delay (unsigned long loops);
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
void printf (const char *fmt, ...);
int sprintf(char * buf, const char *fmt, ...);

#endif
