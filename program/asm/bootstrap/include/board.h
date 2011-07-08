#ifndef __BOARD_H
#define __BOARD_H

#include <config.h>

#define UART0_BASE  0x50000000
/*GPHCON_REGS used to enable UART0/UART1/UART2 */
#define GPHCON   (*(unsigned long volatile *)0x56000070)

#define GPBCON   (*(unsigned long volatile *)0x56000010)
#define GPBDAT   (*(unsigned long volatile *)0x56000014)
#define GPBUP    (*(unsigned long volatile *)0x56000018)

#define pWTCON    0x53000000
#define INTMSK    0x4A000008  /* Interupt-Controller base addresses */
#define INTSUBMSK 0x4A00001C
#define CLKDIVN   0x4C000014  /* clock divisor register */

#define CLK_CTL_BASE    0x4C000000  /* guowenxue */
#define MDIV_405        0x7f << 12  /* guowenxue */
#define PSDIV_405       0x21        /* guowenxue */

#define BEEP     0    /*Buzzer us GPB0 */
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
