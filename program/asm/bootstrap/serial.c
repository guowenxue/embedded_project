/********************************************************************************************
 *        File:  serial.c
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  The UART on board drivers/functions.
 *   ChangeLog:  1, Release initial version on "Tue Jul 12 16:43:18 CST 2011"
 *
 *******************************************************************************************/

#include <common.h>
#define GPHCON   (*(unsigned long volatile *)0x56000070)

void serial_init(void)
{
    S3C24X0_UART *const uart = (S3C24X0_UART *) ELFIN_UART_BASE;

    /*Enable UART0,UART1,UART2 */
    GPHCON = 0x0000aaaa;

    /*Set Parity, Stopbit etc. */
    uart->ULCON &= 0XFFFFFF00;
    uart->ULCON |= 0X03;

    /*Disable modem */
    uart->UMCON = 0x0;

    /*Set UART FIFO control register */
    uart->UFCON = 0x00;
    uart->UCON = 0x805;         /*Enable FIFO */

    /* 
     * UBRDIV = (UART Clock)/(baudrate*16) -1 
     *        = (50 000 000)/(115200*16) -1 
     *        = 26 
     *        = 0x1A
     */
    uart->UBRDIV = 0x1A;
}

void serial_send_byte(char c)
{
    S3C24X0_UART *const uart = (S3C24X0_UART *) ELFIN_UART_BASE;
    /* wait for room in the tx FIFO */
    while (!(uart->UTRSTAT & 0x2)) ;

    uart->UTXH = c;

    if (c == '\n')              /* If \n, also do \r */
        serial_send_byte('\r');
}

int serial_is_recv_enable(void)
{
    S3C24X0_UART *const uart = (S3C24X0_UART *) ELFIN_UART_BASE;
    return uart->UTRSTAT & 0x1;
}

int serial_recv_byte(void)
{
    S3C24X0_UART *const uart = (S3C24X0_UART *) ELFIN_UART_BASE;

    /* wait for character to arrive */
    while (!(uart->UTRSTAT & 0x1)) ;

    return uart->URXH & 0xff;
}

void serial_puts(const char *s)
{
    while (*s)
    {
        serial_send_byte(*s++);
    }
}
