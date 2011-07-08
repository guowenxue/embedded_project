#include <board.h>

#define DELAY_TIME   1000000

inline void delay (unsigned long loops)
{
    __asm__ volatile ("1:\n"
        "subs %0, %1, #1\n"
        "bne 1b":"=r" (loops):"0" (loops));
}

void init_led_beep(void)
{
    /*  Set GPB5,GPB6,GPB8,GPB10 as GPIO mode(0x01) */
    GPBCON = (GPBCON|0x333C03)&0x111401;
    GPBUP = (GPBUP | 0x560);
    /* Set GPB5,GPB6,GPB8,GPB10 as high level, to turn LED0,LED1,LED2,LED3 off */
    GPBDAT = (GPBDAT | 0x560);
}

void turn_led_on(int led)
{
     GPBDAT = (GPBDAT & (~(1<<led)) );
     delay(DELAY_TIME);
}

void turn_led_off(int led)
{
     /* Turn LED0 on */
     GPBDAT = (GPBDAT | (1<<led) );
     delay(DELAY_TIME);
}

void beep(int count)
{
    int   i;
    for(i=0; i<count; i++)
    {
        GPBDAT |= 1<<BEEP;     /* Set Beep GPIO as high level */
        delay(DELAY_TIME*50);

        GPBDAT &= ~(1<<BEEP);  /* Set Beep GPIO as low level */
        delay(DELAY_TIME*50);
    }
}

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


void do_undefined_instruction(void)
{

}

void do_software_interrupt(void)
{
}

void do_prefetch_abort(void)
{
}

void do_data_abort(void)
{
}

void do_not_used(void)
{
}

void do_irq(void)
{
}

void do_fiq(void)
{
}

