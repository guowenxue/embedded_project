/***********************************************************************
 *        File:  led.S
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  This ASM code used to turn LED0~LED4 on/off on FL2440 board
 *   ChangeLog:  1, Release initial version on "Sun Mar 20 18:41:04 CST 2011"
 *
 ***********************************************************************/


#define GPBCON   0x56000010
#define GPBDAT   0x56000014
#define GPBUP    0x56000018

#define OUTPUT   0x01   /*Set GPIO port as output mode*/
#define INPUT    0x00   /*Set GPIO port as input mode*/

#define BEEP     0      /*On FL2440 board, LED0 use GPB0*/
#define LED0     5      /*On FL2440 board, LED0 use GPB5*/
#define LED1     6      /*On FL2440 board, LED0 use GPB6*/
#define LED2     8      /*On FL2440 board, LED0 use GPB8*/
#define LED3     10     /*On FL2440 board, LED0 use GPB10*/

#define DELAY    0X1000000

    .text
    .align 2
    .global _start

_start:
    /*Set GPIO5, GPIO6, GPIO8, GPIO10 as GPIO OUTPUT mode*/
    ldr     r0, =GPBCON
    ldr     r1, [r0]
    bic     r1, r1, #0x3c00      /*Set GPBCON for GPIO5,GPIO6 as 0x00 */
    orr     r1, r1, #0x1400      /*Set GPBCON for GPIO5,GPIO6 as GPIOOUT, 0x01*/
    bic     r1, r1, #0x00330000  /*Set GPBCON for GPIO8,GPIO10 as 0x00*/
    orr     r1, r1, #0x00110000  /*Set GPBCON for GPIO8,GPIO10 as GPIOOUT, 0x01*/
    str     r1, [r0]

    /*Set internal pullup resister*/
    ldr     r0, =GPBUP
    ldr     r1, [r0]
    orr     r1, r1, #0x0560     /*Set bit 5,6,8,10, disable pullup resister*/
    @bic     r1, r1, #0x0560      /*Clear bit 5,6,8,10, enable pullup resister*/
    str     r1, [r0] 


loopled:
    /*Turn off LED5, LED6, LED8, LED10*/
    ldr     r2, =GPBDAT
    ldr     r3, [r2]
    orr     r3, r3, #0x0560     /*Set bit 5,6,8,10 as high level*/
    str     r3, [r2]  
    ldr     r0, =DELAY          /*Sleep for a while*/
    bl      delay

    /*Turn on LED0*/
    ldr     r3, [r2]
    bic     r3, r3, #(1<<LED0)  /*Clear bit 5, set GPB5 as low level*/
    str     r3, [r2]  
    ldr     r0, =DELAY          /*Sleep for a while*/
    bl      delay

    /*Turn on LED1*/
    ldr     r3, [r2]
    bic     r3, r3, #(1<<LED1)  /*Clear bit 6, set GPB6 as low level*/
    str     r3, [r2]  
    ldr     r0, =DELAY          /*Sleep for a while*/
    bl      delay

    /*Turn on LED2*/
    ldr     r3, [r2]
    bic     r3, r3, #(1<<LED2)  /*Clear bit 8, set GPB8 as low level*/
    str     r3, [r2]  
    ldr     r0, =DELAY          /*Sleep for a while*/
    bl      delay

    /*Turn on LED3*/
    ldr     r3, [r2]
    bic     r3, r3, #(1<<LED3)  /*Clear bit 10, set GPB10 as low level*/
    str     r3, [r2]  
    ldr     r0, =DELAY          /*Sleep for a while*/
    bl      delay


    b       loopled             /*Loop running LED*/

delay:
    sub     r0, r0, #1
    cmp     r0, #0x0
    bne     delay
    mov     pc, lr

