
/***********************************************************************
 *        File:  buzzer_led.c
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  This C code used to turn buzzer on/off on FL2440 board
 *   ChangeLog:  1, Release initial version on "Tue Mar 22 21:58:24 CST 2011"
 *
 ***********************************************************************/

#define GPBCON   (*(unsigned long volatile *)0x56000010)
#define GPBDAT   (*(unsigned long volatile *)0x56000014)
#define GPBUP    (*(unsigned long volatile *)0x56000018)

#define BEEP     0    /* Buzzer use GPB0 */
#define LED0     5    /* LED0 use GPB5*/
#define LED1     6    /* LED1 use GPB6*/
#define LED2     8    /* LED2 use GPB8*/
#define LED3     10   /* LED3 use GPB10*/
#define DELAY_TIME   40000000

unsigned int led_gpio[4] = {5,6,8,10};

static inline void delay (unsigned long loops)
{
    __asm__ volatile ("1:\n"
            "subs %0, %1, #1\n"
            "bne 1b":"=r" (loops):"0" (loops));
}

int init_led_buzzer(void)
{
    GPBCON = (GPBCON|0x333C03)&0x111401; /* Set GPB0,GPB5,GPB8,GPB10 as GPIO input mode */
    GPBDAT |= 0x560;   /* Set GPB0,GPB5,GPB6,GPB8,GPB10 as high level */
}

void turn_led_on(int led)
{
    if(led > 3)
        return;

    /* Set GPIO port as low level */
    GPBDAT = (GPBDAT & ( ~(1<<led_gpio[led])) );
}

/* Turn all LED off */
void turn_all_led_off(void)
{
    GPBDAT |= 0x560; /* Set GPIO port as high level */
}

void turn_beep_on(void)
{
    GPBDAT |= 1<<BEEP;     /* Set Beep GPIO as high level, turn beep on */
}

void turn_beep_off(void)
{
    GPBDAT &= ~(1<<BEEP);  /* Set Beep GPIO as low level, turn beep off */
}

void turn_buzzer(int times)
{
    int      i;
    for(i=0; i<times; i++)
    {

        turn_beep_on();
        delay(DELAY_TIME);

        turn_beep_off();
        delay(DELAY_TIME);
    }
}

void turn_led_buzzer_off(void)
{
    turn_all_led_off();
    turn_beep_off();

    return;
}

/* Input value $key is passed by start.S 
 *
 * Key2 pressed -> key=0, turn LED0 on, beep for 1 time
 * Key3 pressed -> key=1, turn LED1 on, beep for 2 time
 * Key4 pressed -> key=2, turn LED2 on, beep for 3 time
 * Key5 pressed -> key=3, turn LED3 on, beep for 4 time
 */

void turn_led_buzzer_on(int key)
{
    turn_led_on(key);    /* Set correspond LED on */
    turn_buzzer(key+1);  /* Let buzzer beep for $key times */
}

