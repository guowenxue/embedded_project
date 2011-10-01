
/********************************************************************************************
 *        File:  start.S - Startup Code for ARM920 CPU-core
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  When system power up, the CPU will comes here to excute the first code here.
 *   ChangeLog:  1, Release initial version on "Tue Jul 12 16:43:18 CST 2011"
 *
 *******************************************************************************************/


/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */

#include <config.h>

.globl _start
_start:	b	start_code
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:	.word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:	.word prefetch_abort
_data_abort:		.word data_abort
_not_used:		.word not_used
_irq:			.word irq
_fiq:			.word fiq

	.balignl 16,0xdeadbeef


/*
 *************************************************************************
 *
 * Startup Code (called from the ARM reset exception vector)
 *
 * do important init only if we don't start from memory!
 * relocate armboot to ram
 * setup stack
 * jump to second stage
 *
 *************************************************************************
 */

_TEXT_BASE:
	.word	TEXT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

/*
 * the actual start code
 */

start_code:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr	r0, r0, #0xd3
	msr	cpsr, r0

	/* Disable watchdog */
    ldr r0, =ELFIN_WATCHDOG_BASE
    mov r1, #0
    str r1, [r0]

	/* Disable Interrupt */
    ldr r0, =ELFIN_INTERRUPT_BASE
    mov r1, #0xffffffff
    str r1, [r0, #INTMSK_OFFSET]
    ldr r1, =0x000007ff
    str r1, [r0, #INTSUBMSK_OFFSET]

	/* flush v4 I/D caches */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/* disable MMU stuff and caches */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 2 (A) Align
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
	mcr	p15, 0, r0, c1, c0, 0


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
    ldr r0, =ELFIN_CLOCK_POWER_BASE
    ldr r1, =0x00ffffff
    str r1, [r0, #LOCKTIME_OFFSET] 

 /*******************************************************************************************
  * Reference to S3C2440 datasheet: Chap 7-8 ~ Page 242
  *
  * Set the selection of Dividing Ratio between FCLK,HCLK and PCLK as FCLK:HCLK:PCLK = 1:4:8.
  * This ratio is determined by HDIVN(here is 2) and PDIVN(here is 1) control register.
  * Refer to the s3c2440 datasheet
  *******************************************************************************************/
    ldr r0, =ELFIN_CLOCK_POWER_BASE
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
    mov r1, #ELFIN_CLOCK_POWER_BASE
    mov r2, #MDIV_405
    add r2, r2, #PSDIV_405
    str r2, [r1, #MPLLCON_OFFSET]

    /* Go to mem_init.S to Init memory controller register */
	bl	mem_init

	/* Set up the stack	*/
stack_setup:
	ldr	r0, =STACK_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CONFIG_SYS_MALLOC_LEN	/* malloc area              */
	sub	r0, r0, #CONFIG_SYS_GBL_DATA_SIZE /* bdinfo                 */
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */
	bic	sp, sp, #7		/* 8-byte alignment for ABI compliance */

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	ldr	r1, _bss_end		/* stop here                        */
	mov	r2, #0x00000000		/* clear                            */

clbss_l:str	r2, [r0]		/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l

#if 0  /*Don't wanna init BEEP & LED here*/
    bl  init_led_beep

    /*
     * R0,R1,R2 are the 1st,2nd, 3rd argument passed to C function
     * If need pass more than 3 arguments, then we need use stack.
     */
    mov r0,#LED1    
    bl  turn_led_on
#endif

    bl  bootstrap_main

/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */

@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC	0x13
#define I_BIT		0x80

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
 */

	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	ldr	r2, _armboot_start
	sub	r2, r2, #(CONFIG_STACKSIZE)
	sub	r2, r2, #(CONFIG_SYS_MALLOC_LEN)
	/* set base 2 words into abort stack */
	sub	r2, r2, #(CONFIG_SYS_GBL_DATA_SIZE+8)
	ldmia	r2, {r2 - r3}			@ get pc, cpsr
	add	r0, sp, #S_FRAME_SIZE		@ restore sp_SVC

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r3}			@ save sp_SVC, lr_SVC, pc, cpsr
	mov	r0, sp
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	add	r7, sp, #S_PC
	stmdb	r7, {sp, lr}^			@ Calling SP, LR
	str	lr, [r7, #0]			@ Save calling PC
	mrs	r6, spsr
	str	r6, [r7, #4]			@ Save CPSR
	str	r0, [r7, #8]			@ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	/* return & move spsr_svc into cpsr */
	subs	pc, lr, #4
	.endm

	.macro get_bad_stack
	ldr	r13, _armboot_start		@ setup our mode stack
	sub	r13, r13, #(CONFIG_STACKSIZE)
	sub	r13, r13, #(CONFIG_SYS_MALLOC_LEN)
	/* reserve a couple spots in abort stack */
	sub	r13, r13, #(CONFIG_SYS_GBL_DATA_SIZE+8)

	str	lr, [r13]			@ save caller lr / spsr
	mrs	lr, spsr
	str	lr, [r13, #4]

	mov	r13, #MODE_SVC			@ prepare SVC-Mode
	@ msr	spsr_c, r13
	msr	spsr, r13
	mov	lr, pc
	movs	pc, lr
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm

/*
 * exception handlers
 */
	.align  5
undefined_instruction:
	get_bad_stack
	bad_save_user_regs
	bl	do_undefined_instruction

	.align	5
software_interrupt:
	get_bad_stack
	bad_save_user_regs
	bl	do_software_interrupt

	.align	5
prefetch_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_prefetch_abort

	.align	5
data_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_data_abort

	.align	5
not_used:
	get_bad_stack
	bad_save_user_regs
	bl	do_not_used

	.align	5
irq:
	get_bad_stack
	bad_save_user_regs
	bl	do_irq

	.align	5
fiq:
	get_bad_stack
	bad_save_user_regs
	bl	do_fiq
