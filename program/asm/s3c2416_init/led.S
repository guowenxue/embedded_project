/***********************************************************************
 *        File:  led.S
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  This ASM code used to turn LED1,LED2 on/off on S3C2416 board
 *   ChangeLog:  1, Release initial version on "Sun Mar 20 18:41:04 CST 2011"
 *
 ***********************************************************************/


/*GPIO for LED register*/
#define GPACON                  0x56000000
#define GPADAT                  0x56000004
#define LED1                    1  /*On S3C2416 board, LED1 use GPA1*/
#define LED2                    2  /*On S3C2416 board, LED2 use GPA2*/
#define DELAY                   0X1000000

    .text
    .align 2
    .global _start

_start:

    /*Set GPACON for GPA1,GPA2 as 0 to GPIO mode */
    ldr r0, =GPACON
    ldr r1, [r0]
    bic r1, r1, #0x6
    str r1, [r0]

    /*Turn off LED1, LED2*/
    ldr r0, =GPADAT
    ldr r1, [r0]
    orr r1, r1, #0x6     /*Set bit 1,2 as high level*/
    str r1, [r0]

loopled:

    /*Turn on LED1*/
    ldr r0, =GPADAT
    ldr r1, [r0]
    orr r1, r1, #0x6     /*Set bit 1,2 as high level*/
    bic r1, r1, #(1<<LED1)  /*Set GPA1 as low level, turn on LED1*/
    str r1, [r0]
    ldr     r2, =DELAY
    bl      delay

    /*Turn on LED2*/
    ldr r0, =GPADAT
    ldr r1, [r0]
    orr r1, r1, #0x6     /*Set bit 1,2 as high level*/
    bic r1, r1, #(1<<LED2)  /*Set GPA2 as low level, turn on LED2*/
    str r1, [r0]
    ldr     r2, =DELAY
    bl      delay

    b       loopled             /*Loop running LED*/

delay:
    sub     r2, r2, #1
    cmp     r2, #0x0
    bne     delay
    mov     pc, lr

