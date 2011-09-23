
/********************************************************************************************
 *        File:  init.S 
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <Email: guowenxue@gmail.com QQ:281143292>
 * Description:  If we wanna debug u-boot by J-Link in external SDRAM, we must download this 
 *               init.bin file into s3c24x0 8K internal SRAM(Stepping Stone) and excute first,
 *               which used to initialize the CPU and external SDRAM. Only after initialize 
 *               the SDRAM then we can debug u-boot in it.
 *   ChangeLog:  1, Release initial version on "Tue Jul 12 16:43:18 CST 2011"
 *
 *******************************************************************************************/

#include "init.h"

    .text
    .align 2
    .global _start

_start:
    /* set the cpu to SVC32 mode */
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0xd3
    msr cpsr, r0

    /* flush v4 I/D caches */
    mov r0, #0
    mcr p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
    mcr p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

    /* disable MMU stuff and caches */
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
    bic r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM) 
    orr r0, r0, #0x00000002	@ set bit 2 (A) Align
    orr r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
    mcr p15, 0, r0, c1, c0, 0

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

    /* Disable watchdog */
    ldr r0, =S3C_WATCHDOG_BASE
    mov r1, #0
    str r1, [r0]

    /* Disable Interrupt */
    ldr r0, =S3C_INTERRUPT_BASE
    mvn r1, #0x0   /* equal to 'mov r1 0xFFFFFFFF' */
    str r1, [r0, #INTMSK_OFFSET]
    str r1, [r0, #INTSUBMSK_OFFSET]

    /* Set all interrupts as IRQ */
    mov r1, #0x0
    str r1, [r0, #INTMOD_OFFSET]

    /* system_clock_init: Initialize core clock and bus clock. */
    ldr r0, =S3C_CLOCK_POWER_BASE

    ldr r1, [r0, #CLKDIV0CON_OFFSET]
    bic r1, r1, #0x37   /* clear HCLKDIV, PREDIV, PCLKDIV */
    bic r1, r1, #(0xf<<9) /* clear ARMCLKDIV */
    ldr r2, =CLK_DIV_VAL
    orr r1, r1, r2
    str r1, [r0, #CLKDIV0CON_OFFSET]

    /*Set MPLL Locktime*/
    ldr r1, =0xe10
    str r1, [r0, #LOCKCON0_OFFSET]

    ldr r1, =MPLL_VAL
    str r1, [r0, #MPLLCON_OFFSET]

    ldr r1, =0x1780
    str r1, [r0, #LOCKCON1_OFFSET]

    ldr r1, =EPLL_VAL       /*Set EPLL*/
    str r1, [r0, #EPLLCON_OFFSET]

    ldr r1, [r0, #CLKSRCCON_OFFSET]
    orr r1, r1, #0x40
    str r1, [r0, #CLKSRCCON_OFFSET]

    /* set CLKSRC to fit MPLL and USB */
    ldr r1, [r0, #CLKSRCCON_OFFSET]
    orr r1, r1, #0x10
    str r1, [r0, #CLKSRCCON_OFFSET]

    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #0xc0000000 /* clock setting in MMU */
    mcr p15, 0, r0, c1, c0, 0


mem_init:
    /* memory control configuration */
    /* make r0 relative the current location so that it */
    /* reads SMRDATA out of FLASH rather than memory ! */
    ldr r0, =SMRDATA
    ldr r1, =mem_init
    sub r0, r0, r1
    adr r3, mem_init       /* r3 <- current position of code   */
    add r0, r0, r3  
    ldr r1, =S3C_MEMCTL_BASE 
    add r2, r0, #13*4
0:
    ldr r3, [r0], #4
    str r3, [r1], #4
    cmp r2, r0
    bne 0b

    ldr r4, =S3C_MEMCTL_BASE

    /* Step 1: BANKCFG Setting */
    ldr r2, =CFG_BANK_CFG_VAL_DDR2
    str r2, [r4, #BANKCFG_OFFSET]

    /*Setp 2: BANKCON1/2 Setting */
    ldr r2, =0x44000040
    str r2, [r4, #BANKCON1_OFFSET]

    ldr r2, =CFG_BANK_CON2_VAL_DDR2
    str r2, [r4, #BANKCON2_OFFSET]

    /* Step 3: issue PALL */
    orr r2, r1, #INIT_PALL
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 4: Issue a EMRS2 command */
    ldr r2, =0x80000000
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_EMRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 5: Issue a EMRS3 command */
    ldr r2, =0xc0000000
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_EMRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 6: Issue a EMRS1 command */
    ldr r2, =0x44000000
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_EMRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 7: issue MRS */
    ldr r2, =0x44000130
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_MRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 8: issue PALL */
    orr r2, r1, #INIT_PALL
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 9: write 0xff into the refresh timer */
    mov r3, #0xff
    str r3, [r4, #REFRESH_OFFSET]

    /* Step 10: wait more than 120 clk */
    mov r3, #0x100
10: subs    r3, r3, #1
    bne 10b

    /* Step 11: issue MRS */
    ldr r2, =0x44000030
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_MRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 12: Issue a EMRS1 command */
    ldr r2, =0x47800030
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_EMRS
    str r2, [r4, #BANKCON1_OFFSET]

    ldr r2, =0x44000030
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_EMRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 13: write 0x87 into the refresh timer */
    mov r3, #0x87
    str r3, [r4, #REFRESH_OFFSET]

    /* Step 14: Normal Mode */
    orr r2, r1, #INIT_NORMAL
    str r2, [r4, #BANKCON1_OFFSET]

    ldr r6, =0x30000000
    ldr r7, =0x24564236
    swp r8, r7, [r6]
    swp r5, r8, [r6]
    cmp r7, r5

    bne dead_loop

    ldr r0, =GPADAT
    ldr r1, [r0]
    orr r1, r1, #0x6     /*Set bit 1,2 as high level*/
    bic r1, r1, #(1<<LED2)  /*Set GPA2 as low level, turn on LED2*/
    str r1, [r0]

    /* everything is fine now */
dead_loop:
    b   dead_loop

    .ltorg
/* the literal pools origin */

SMRDATA:
    .word   CFG_BANK_CFG_VAL
    .word   CFG_BANK_CON1_VAL
    .word   CFG_BANK_CON2_VAL
    .word   CFG_BANK_CON3_VAL
    .word   CFG_BANK_REFRESH_VAL
