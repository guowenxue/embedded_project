/********************************************************************************************
 *        File:  led_beep.c
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  LED & Beep operate funciton.
 *   ChangeLog:  1, Release initial version on "Tue Jul 12 16:43:18 CST 2011"
 *
 *******************************************************************************************/

#include <common.h>
#define DELAY_TIME   1000000

/*LED & Beep */
#define GPBCON   (*(unsigned long volatile *)0x56000010)
#define GPBDAT   (*(unsigned long volatile *)0x56000014)
#define GPBUP    (*(unsigned long volatile *)0x56000018)

inline void delay(unsigned long loops)
{
    __asm__ volatile ("1:\n" "subs %0, %1, #1\n" "bne 1b":"=r" (loops):"0"(loops));
}

void init_led_beep(void)
{
    /*  Set GPB5,GPB6,GPB8,GPB10 as GPIO mode(0x01) */
    GPBCON = (GPBCON | 0x333C03) & 0x111401;
    GPBUP = (GPBUP | 0x560);
    /* Set GPB5,GPB6,GPB8,GPB10 as high level, to turn LED0,LED1,LED2,LED3 off */
    GPBDAT = (GPBDAT | 0x560);
}

void turn_led_on(int led)
{
    GPBDAT = (GPBDAT & (~(1 << led)));
    delay(DELAY_TIME);
}

void turn_led_off(int led)
{
    /* Turn LED0 on */
    GPBDAT = (GPBDAT | (1 << led));
    delay(DELAY_TIME);
}

void beep(int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        GPBDAT |= 1 << BEEP;    /* Set Beep GPIO as high level */
        delay(DELAY_TIME * 50);

        GPBDAT &= ~(1 << BEEP); /* Set Beep GPIO as low level */
        delay(DELAY_TIME * 50);
    }
}
