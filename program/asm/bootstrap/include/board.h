/********************************************************************************************
 *        File:  board.h
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  Head file for the board configure
 *   ChangeLog:  1, Release initial version on "Tue Jul 12 16:43:18 CST 2011"
 *
 *******************************************************************************************/

#ifndef __BOARD_H
#define __BOARD_H

#include <config.h>

#define UART0_BASE  0x50000000
/*GPHCON_REGS used to enable UART0/UART1/UART2 */
#define GPHCON   (*(unsigned long volatile *)0x56000070)

/*Watchdog and interrupt*/
#define ELFIN_WATCHDOG_BASE 0x53000000 
#define WTCON_OFFSET        0x00
#define WTDAT_OFFSET        0x04
#define WTCNT_OFFSET        0x08

#define ELFIN_INTERRUPT_BASE    0x4a000000
#define SRCPND_OFFSET       0x00
#define INTMOD_OFFSET       0x04
#define INTMSK_OFFSET       0x08
#define PRIORITY_OFFSET     0x0c
#define INTPND_OFFSET       0x10
#define INTOFFSET_OFFSET    0x14
#define SUBSRCPND_OFFSET    0x18
#define INTSUBMSK_OFFSET    0x1c

/*Clock and power*/
#define ELFIN_CLOCK_POWER_BASE  0x4c000000
  
#define LOCKTIME_OFFSET     0x00
#define MPLLCON_OFFSET      0x04
#define UPLLCON_OFFSET      0x08
#define CLKCON_OFFSET       0x0c
#define CLKSLOW_OFFSET      0x10
#define CLKDIVN_OFFSET      0x14
#define CAMDIVN_OFFSET      0x18

#define MDIV_405        0x7f << 12
#define PSDIV_405       0x21

/*LED & Beep */
#define GPBCON   (*(unsigned long volatile *)0x56000010)
#define GPBDAT   (*(unsigned long volatile *)0x56000014)
#define GPBUP    (*(unsigned long volatile *)0x56000018)

#define BEEP     0    /*Buzzer use GPB0 */
#define LED0     5    /*LED0 use GPB5*/
#define LED1     6    /*LED1 use GPB6*/
#define LED2     8    /*LED2 use GPB8*/
#define LED3     10   /*LED3 use GPB10*/

#ifndef __ASSEMBLY__
typedef volatile unsigned char  S3C24X0_REG8;
typedef volatile unsigned short S3C24X0_REG16;
typedef volatile unsigned int   S3C24X0_REG32;

/* UART (see manual chapter 11) */
typedef struct {
    S3C24X0_REG32   ULCON;
    S3C24X0_REG32   UCON;
    S3C24X0_REG32   UFCON;
    S3C24X0_REG32   UMCON;
    S3C24X0_REG32   UTRSTAT;
    S3C24X0_REG32   UERSTAT;
    S3C24X0_REG32   UFSTAT;
    S3C24X0_REG32   UMSTAT;
    S3C24X0_REG8    UTXH;
    S3C24X0_REG8    res1[3];
    S3C24X0_REG8    URXH;
    S3C24X0_REG8    res2[3];
    S3C24X0_REG32   UBRDIV;
} /*__attribute__((__packed__))*/ S3C24X0_UART;

#endif  /*Not define __ASSEMBLY__ */

#endif  /*Not define __BOARD_H__*/
