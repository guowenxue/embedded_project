/***********************************************************************
 *        File:  start.S
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  This ASM used to detect the four keys status, if it's pressed down
 *               then call the C function to turn the buzzer beep and LED on
 *   ChangeLog:  1, Release initial version on "Tue Mar 22 21:58:24 CST 2011"
 *
 ***********************************************************************/

#define pWTCON    0x53000000  /* Watch dog register address */
#define INTMSK    0x4A000008  /* Interupt-Controller base addresses */
#define INTSUBMSK 0x4A00001C

#define GPFCON    0x56000050
#define GPFDAT    0x56000054
#define GPFUP     0x56000058

#define KEY2      0  /*KEY S2 use GPF0/EINT0 port*/
#define KEY3      2  /*KEY S3 use GPF2/EINT2 port*/
#define KEY4      3  /*KEY S4 use GPF3/EINT3 port*/
#define KEY5      4  /*KEY S5 use GPF4/EINT4 port*/

    .text
    .align 2
    .global _start

_start:

    /* Disable watch dog */
    ldr r0, =pWTCON  /*Save pwTCON address in r0*/
    mov r1, #0x0     /*Set r1=0x0*/
    str r1, [r0]     /*Move the data in r1 to the address specify by r0*/

    /* mask all IRQs by setting all bits in the INTMR - default */
    mov r1, #0xffffffff
    ldr r0, =INTMSK
    str r1, [r0] 

    ldr r0, =INTSUBMSK
    ldr r1, =0x7fff    /*There are 15 bits used in INTSUBMSK on S3C2440*/
    str r1, [r0]

    /*Set Beep GPIO port as GPIO input mode*/
    ldr r0, =GPFCON  
    ldr r1, [r0]
    bic r1, r1, #0xf3    /* Set GPF0, GPF2, GPF3, as GPIO input mode(00)*/
    bic r1, r1, #0x0300  /* Set GPF4 as GPIO input mode(00)*/
    str r1, [r0]

    bl init_led_buzzer  /*Call C function to init buzzer and LED GPIO port*/

loop_detect:
    bl turn_led_buzzer_off

    ldr r1, =GPFDAT   /*Read the four key GPIO data port*/
    ldr r2, [r1]

    /*Check Key2 pressed down or not*/
    mov r0, #0  /*Turn LED0 on*/
    tst r2, #0x1
    bleq led_beep_on

    /*Check Key3 pressed down or not*/
    mov r0, #1  /*Turn LED1 on*/
    tst r2, #0x4
    bleq led_beep_on

    /*Check Key4 pressed down or not*/
    mov r0, #2  /*Turn LED2 on*/
    tst r2, #0x8
    bleq led_beep_on

    /*Check Key5 pressed down or not*/
    mov r0, #3  /*Turn LED3 on*/
    tst r2, #0x10
    bleq led_beep_on

    /*No key pressed, detect again*/
    b loop_detect

led_beep_on:
    bl turn_led_buzzer_on  /*Turn buzzer beep*/
    @mov pc, lr    /*Why mov lr, pc can not work?*/
    b loop_detect


    /*Won't come here*/
halt_loop:
    b halt_loop

