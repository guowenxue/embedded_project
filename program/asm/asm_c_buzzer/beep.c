
/***********************************************************************
 *        File:  beep.c
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  This C code used to turn buzzer on/off on FL2440 board
 *   ChangeLog:  1, Release initial version on "Mon Mar 21 21:09:52 CST 2011"
 *
 ***********************************************************************/

#define GPBCON   (*(unsigned long volatile *)0x56000010)
#define GPBDAT   (*(unsigned long volatile *)0x56000014)
#define GPBUP    (*(unsigned long volatile *)0x56000018)

#define BEEP         0    /*Buzzer us GPB0 */
#define LED0         5
#define DELAY_TIME   40000000

static inline void delay (unsigned long loops)
{
    __asm__ volatile ("1:\n"
            "subs %0, %1, #1\n"
            "bne 1b":"=r" (loops):"0" (loops));
}

int main(void)
{
    GPBCON = (GPBCON|0x3)&0x1; /* Set GPB0 as GPIO output mode(0x01) */
    GPBUP &= ~1;               /* Enable pullup resister */

    GPBDAT |= 0x560;
    while(1)
    {
        GPBDAT &= ~(1<<BEEP);  /* Set Beep GPIO as low level */
        delay(DELAY_TIME);

        GPBDAT |= 1<<BEEP;     /* Set Beep GPIO as high level */
        delay(DELAY_TIME);
    }
}

