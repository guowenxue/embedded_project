
#include <board.h>

void serial_init(void)
{   
    S3C24X0_UART *const uart = (S3C24X0_UART *)UART0_BASE;

    /*Enable UART0,UART1,UART2*/
    GPHCON = 0x0000aaaa; 

    /*Set Parity, Stopbit etc.*/
    uart->ULCON &=0XFFFFFF00;
    uart->ULCON |=0X03;  

    /*Disable modem*/
    uart->UMCON = 0x0;  

    /*Set UART FIFO control register*/
    uart->UFCON = 0x00;
    uart->UCON = 0x805; /*Enable FIFO*/

    /* 
     * UBRDIV = (UART Clock)/(baudrate*16) -1 
     *        = (50 000 000)/(115200*16) -1 
     *        = 26 
     *        = 0x1A
     */
    uart->UBRDIV = 0x1A;  
}

void serial_putc(char c)
{
    S3C24X0_UART *const uart = (S3C24X0_UART *)UART0_BASE;
    /* wait for room in the tx FIFO */
    while (!(uart->UTRSTAT & 0x2));

    uart->UTXH = c;

    if (c == '\n')   /* If \n, also do \r */
        serial_putc('\r');
}

void serial_puts(const char *s)
{
     while (*s) {
         serial_putc(*s++);
     }
}

