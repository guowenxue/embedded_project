/***********************************************************************
 *        File:  start.S
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  This ASM used to disable watch dog and interrupt, then call C code to 
 *               turn the buzzer on/off on FL2440 board.
 *   ChangeLog:  1, Release initial version on "Mon Mar 21 21:09:52 CST 2011"
 *
 ***********************************************************************/

#define pWTCON    0x53000000  /* Watch dog register address */
#define INTMSK    0x4A000008  /* Interupt-Controller base addresses */
#define INTSUBMSK 0x4A00001C

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

    bl main

halt_loop:
    b halt_loop

