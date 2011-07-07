/***********************************************************************
 *        File:  start.S
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  This ASM used to disable watch dog and interrupt, then call C code to 
 *               turn the four LEDs on/off on FL2440 board.
 *   ChangeLog:  1, Release initial version on "Mon Mar 21 21:09:52 CST 2011"
 *
 ***********************************************************************/

#define pWTCON    0x53000000  /* Watch dog register address */
#define INTMSK    0x4A000008  /* Interupt-Controller base addresses */
#define INTSUBMSK 0x4A00001C

#define LED0     5    /*LED0 use GPB5*/
#define LED1     6    /*LED1 use GPB6*/
#define LED2     8    /*LED2 use GPB8*/
#define LED3     10   /*LED3 use GPB10*/


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
    
    bl  led_init

loop:
    mov r0,#LED0
    bl turn_led_on

    mov r0,#LED1
    bl turn_led_on

    mov r0,#LED2
    bl turn_led_on

    mov r0,#LED3
    bl turn_led_on

    mov r0,#LED0
    bl turn_led_off

    mov r0,#LED1
    bl turn_led_off

    mov r0,#LED2
    bl turn_led_off

    mov r0,#LED3
    bl turn_led_off

    b loop

    @bl main

halt_loop:
    b halt_loop

