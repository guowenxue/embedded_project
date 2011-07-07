
/***********************************************************************
 *        File:  led.c
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  This C code used to turn LED0~LED4 on on FL2440 board
 *   ChangeLog:  1, Release initial version on "Mon Mar 21 21:09:52 CST 2011"
 *
 ***********************************************************************/

#define GPBCON   (*(unsigned long volatile *)0x56000010)
#define GPBDAT   (*(unsigned long volatile *)0x56000014)
#define GPBUP    (*(unsigned long volatile *)0x56000018)

#define LED0     5    /*LED0 use GPB5*/
#define LED1     6    /*LED1 use GPB6*/
#define LED2     8    /*LED2 use GPB8*/
#define LED3     10   /*LED3 use GPB10*/

#define DELAY_TIME   1000000

static inline void delay (unsigned long loops)
{
    __asm__ volatile ("1:\n"
        "subs %0, %1, #1\n"
        "bne 1b":"=r" (loops):"0" (loops));
}

void led_init(void)
{
    /*  Set GPB5,GPB6,GPB8,GPB10 as GPIO mode(0x01) */ 
    GPBCON = (GPBCON|0x333C00)&0x111400;
    GPBUP = (GPBUP | 0x560);
    /* Set GPB5,GPB6,GPB8,GPB10 as high level, to turn LED0,LED1,LED2,LED3 off */
    GPBDAT = (GPBDAT | 0x560);
}

int main(void)
{
    led_init();
    while(1)
    {
        /* Set GPB5,GPB6,GPB8,GPB10 as high level, to turn LED0,LED1,LED2,LED3 off */
        GPBDAT = (GPBDAT | 0x560);
        delay(DELAY_TIME);

        /* Turn LED0 on */
        GPBDAT = (GPBDAT & (~(1<<LED0)) );
        delay(DELAY_TIME);

        /* Turn LED1 on */
        GPBDAT = (GPBDAT & (~(1<<LED1)) );
        delay(DELAY_TIME);

        /* Turn LED2 on */
        GPBDAT = (GPBDAT & (~(1<<LED2)) );
        delay(DELAY_TIME);

        /* Turn LED3 on */
        GPBDAT = (GPBDAT & (~(1<<LED3)) );
        delay(DELAY_TIME);
    }
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

