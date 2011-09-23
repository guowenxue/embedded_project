
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

    /* Disable watchdog */
    ldr r0, =S3C_WATCHDOG_BASE
    mov r1, #0
    str r1, [r0]

    /* Disable Interrupt */
    ldr r0, =S3C_INTERRUPT_BASE
    mov r1, #0xffffffff
    str r1, [r0, #INTMSK_OFFSET]
    ldr r1, =0x000007ff
    str r1, [r0, #INTSUBMSK_OFFSET]

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


 /*******************************************************************************************
  * Init system clock and power, FCLK:HCLK:PCLK = 1:4:8 
  * Reference to S3C2440 datasheet: Chap 7 Clock&Power Management
  *
  * Initialize System Clock FCLK=400MHz HCLK=100MHz PCLK=50MHz
  * FCLK is used by ARM920T
  * HCLK is used for AHB bus, which is used by the ARM920T, the memory controller,
  *      the interrupt controller, the LCD controller, the DMA and USB host block.
  * PCLK is is used for APB bus,which is used by the peripherals such as WDT,IIS,I2C,
  *      PWM timer,MMC interface,ADC,UART,GPIO,RTC and SPI.
  ******************************************************************************************/

    /*Set LOCKTIME as default value 0x00ffffff*/
    ldr r0, =S3C_CLOCK_POWER_BASE
    ldr r1, =0x00ffffff
    str r1, [r0, #LOCKTIME_OFFSET] 

 /*******************************************************************************************
  * Reference to S3C2440 datasheet: Chap 7-8 ~ Page 242
  *
  * Set the selection of Dividing Ratio between FCLK,HCLK and PCLK as FCLK:HCLK:PCLK = 1:4:8.
  * This ratio is determined by HDIVN(here is 2) and PDIVN(here is 1) control register.
  * Refer to the s3c2440 datasheet
  *******************************************************************************************/
    ldr r0, =S3C_CLOCK_POWER_BASE
    mov r1, #5
    str r1, [r0, #CLKDIVN_OFFSET]  /*Set Clock Divider*/

    mrc p15, 0, r1, c1, c0, 0 
    orr r1, r1, #0xc0000000
    mcr p15, 0, r1, c1, c0, 0 

 /***************************************************************************************
  *  Reference to S3C2440 datasheet: Chap 7-20 ~ Page 254
  *
  *  Set MPLLCON(0x4C000004) register as:
  *    [19:12]:  MDIV(Main Divider control)=0x7F (value set in MDIV_405)
  *      [9:4]:  PDIV(Pre-devider control)=0x02  (value set in PSDIV_405)
  *      [1:0]:  SDIV(Post divider control)=0x01 (value set in PSDIV_405)
  *
  *  MPLL(FCLK) = (2 * m * Fin)/(p * 2^s)
  *  m=(MDIV+8), p=(PDIV+2), s=SDIV
  *
  *  So FCLK=((2*(127+8)*Fin)) / ((2+2)*2^1)
  *         = (2*135*12MHz)/8
  *         = 405MHz
  *  For FCLK:HCLK:PCLK=1:4:8, so HCLK=100MHz, PCLK=50MHz
  ***************************************************************************************/
    mov r1, #S3C_CLOCK_POWER_BASE
    mov r2, #MDIV_405
    add r2, r2, #PSDIV_405
    str r2, [r1, #MPLLCON_OFFSET]

mem_init:
    /* memory control configuration */
    /* make r0 relative the current location so that it */
    /* reads SMRDATA out of FLASH rather than memory ! */
    ldr r0, =SMRDATA
    ldr r1, =mem_init
    sub r0, r0, r1
    adr r3, mem_init       /* r3 <- current position of code   */
    add r0, r0, r3  /*r0 =SMRDATA-mem_init+mem_init =SMRDATA*/
    ldr r1, =BWSCON /* Bus Width Status Controller */
    add r2, r0, #13*4
0:
    ldr r3, [r0], #4
    str r3, [r1], #4
    cmp r2, r0
    bne 0b

    /* everything is fine now */
dead_loop:
    b   dead_loop

    .ltorg
/* the literal pools origin */

SMRDATA:
    .word (0+(B1_BWSCON<<4)+(B2_BWSCON<<8)+(B3_BWSCON<<12)+(B4_BWSCON<<16)+(B5_BWSCON<<20)+(B6_BWSCON<<24)+(B7_BWSCON<<28))
    .word ((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC))
    .word ((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC))
    .word ((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC))
    .word ((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC))
    .word ((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC))
    .word ((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC))
    .word ((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))
    .word ((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))
    .word ((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Trc<<18)+(Tchr<<16)+REFCNT)
    .word 0xb2
    .word 0x30
    .word 0x30
