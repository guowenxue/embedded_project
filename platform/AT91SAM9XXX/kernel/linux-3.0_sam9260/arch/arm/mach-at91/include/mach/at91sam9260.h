/*
 * arch/arm/mach-at91/include/mach/at91sam9260.h
 *
 * (C) 2006 Andrew Victor
 *
 * Common definitions.
 * Based on AT91SAM9260 datasheet revision A (Preliminary).
 *
 * Includes also definitions for AT91SAM9XE and AT91SAM9G families
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef AT91SAM9260_H
#define AT91SAM9260_H

/*
 * Peripheral identifiers/interrupts.
 */
#define AT91SAM9260_ID_PIOA	2	/* Parallel IO Controller A */
#define AT91SAM9260_ID_PIOB	3	/* Parallel IO Controller B */
#define AT91SAM9260_ID_PIOC	4	/* Parallel IO Controller C */
#define AT91SAM9260_ID_ADC	5	/* Analog-to-Digital Converter */
#define AT91SAM9260_ID_US0	6	/* USART 0 */
#define AT91SAM9260_ID_US1	7	/* USART 1 */
#define AT91SAM9260_ID_US2	8	/* USART 2 */
#define AT91SAM9260_ID_MCI	9	/* Multimedia Card Interface */
#define AT91SAM9260_ID_UDP	10	/* USB Device Port */
#define AT91SAM9260_ID_TWI	11	/* Two-Wire Interface */
#define AT91SAM9260_ID_SPI0	12	/* Serial Peripheral Interface 0 */
#define AT91SAM9260_ID_SPI1	13	/* Serial Peripheral Interface 1 */
#define AT91SAM9260_ID_SSC	14	/* Serial Synchronous Controller */
#define AT91SAM9260_ID_TC0	17	/* Timer Counter 0 */
#define AT91SAM9260_ID_TC1	18	/* Timer Counter 1 */
#define AT91SAM9260_ID_TC2	19	/* Timer Counter 2 */
#define AT91SAM9260_ID_UHP	20	/* USB Host port */
#define AT91SAM9260_ID_EMAC	21	/* Ethernet */
#define AT91SAM9260_ID_ISI	22	/* Image Sensor Interface */
#define AT91SAM9260_ID_US3	23	/* USART 3 */
#define AT91SAM9260_ID_US4	24	/* USART 4 */
#define AT91SAM9260_ID_US5	25	/* USART 5 */
#define AT91SAM9260_ID_TC3	26	/* Timer Counter 3 */
#define AT91SAM9260_ID_TC4	27	/* Timer Counter 4 */
#define AT91SAM9260_ID_TC5	28	/* Timer Counter 5 */
#define AT91SAM9260_ID_IRQ0	29	/* Advanced Interrupt Controller (IRQ0) */
#define AT91SAM9260_ID_IRQ1	30	/* Advanced Interrupt Controller (IRQ1) */
#define AT91SAM9260_ID_IRQ2	31	/* Advanced Interrupt Controller (IRQ2) */


/*
 * User Peripheral physical base addresses.
 */
#define AT91SAM9260_BASE_TCB0		0xfffa0000
#define AT91SAM9260_BASE_TC0		0xfffa0000
#define AT91SAM9260_BASE_TC1		0xfffa0040
#define AT91SAM9260_BASE_TC2		0xfffa0080
#define AT91SAM9260_BASE_UDP		0xfffa4000
#define AT91SAM9260_BASE_MCI		0xfffa8000
#define AT91SAM9260_BASE_TWI		0xfffac000
#define AT91SAM9260_BASE_US0		0xfffb0000
#define AT91SAM9260_BASE_US1		0xfffb4000
#define AT91SAM9260_BASE_US2		0xfffb8000
#define AT91SAM9260_BASE_SSC		0xfffbc000
#define AT91SAM9260_BASE_ISI		0xfffc0000
#define AT91SAM9260_BASE_EMAC		0xfffc4000
#define AT91SAM9260_BASE_SPI0		0xfffc8000
#define AT91SAM9260_BASE_SPI1		0xfffcc000
#define AT91SAM9260_BASE_US3		0xfffd0000
#define AT91SAM9260_BASE_US4		0xfffd4000
#define AT91SAM9260_BASE_US5		0xfffd8000
#define AT91SAM9260_BASE_TCB1		0xfffdc000
#define AT91SAM9260_BASE_TC3		0xfffdc000
#define AT91SAM9260_BASE_TC4		0xfffdc040
#define AT91SAM9260_BASE_TC5		0xfffdc080
#define AT91SAM9260_BASE_ADC		0xfffe0000
#define AT91_BASE_SYS			0xffffe800

/*
 * System Peripherals (offset from AT91_BASE_SYS)
 */
#define AT91_ECC	(0xffffe800 - AT91_BASE_SYS)
#define AT91_SDRAMC0	(0xffffea00 - AT91_BASE_SYS)
#define AT91_SMC	(0xffffec00 - AT91_BASE_SYS)
#define AT91_MATRIX	(0xffffee00 - AT91_BASE_SYS)
#define AT91_CCFG	(0xffffef10 - AT91_BASE_SYS)
#define AT91_AIC	(0xfffff000 - AT91_BASE_SYS)
#define AT91_DBGU	(0xfffff200 - AT91_BASE_SYS)
#define AT91_PIOA	(0xfffff400 - AT91_BASE_SYS)
#define AT91_PIOB	(0xfffff600 - AT91_BASE_SYS)
#define AT91_PIOC	(0xfffff800 - AT91_BASE_SYS)
#define AT91_PMC	(0xfffffc00 - AT91_BASE_SYS)
#define AT91_RSTC	(0xfffffd00 - AT91_BASE_SYS)
#define AT91_SHDWC	(0xfffffd10 - AT91_BASE_SYS)
#define AT91_RTT	(0xfffffd20 - AT91_BASE_SYS)
#define AT91_PIT	(0xfffffd30 - AT91_BASE_SYS)
#define AT91_WDT	(0xfffffd40 - AT91_BASE_SYS)
#define AT91_GPBR	(0xfffffd50 - AT91_BASE_SYS)

#define AT91_USART0	AT91SAM9260_BASE_US0
#define AT91_USART1	AT91SAM9260_BASE_US1
#define AT91_USART2	AT91SAM9260_BASE_US2
#define AT91_USART3	AT91SAM9260_BASE_US3
#define AT91_USART4	AT91SAM9260_BASE_US4
#define AT91_USART5	AT91SAM9260_BASE_US5


/*
 * Internal Memory.
 */
#define AT91SAM9260_ROM_BASE	0x00100000	/* Internal ROM base address */
#define AT91SAM9260_ROM_SIZE	SZ_32K		/* Internal ROM size (32Kb) */

#define AT91SAM9260_SRAM0_BASE	0x00200000	/* Internal SRAM 0 base address */
#define AT91SAM9260_SRAM0_SIZE	SZ_4K		/* Internal SRAM 0 size (4Kb) */
#define AT91SAM9260_SRAM1_BASE	0x00300000	/* Internal SRAM 1 base address */
#define AT91SAM9260_SRAM1_SIZE	SZ_4K		/* Internal SRAM 1 size (4Kb) */

#define AT91SAM9260_UHP_BASE	0x00500000	/* USB Host controller */

#define AT91SAM9XE_FLASH_BASE	0x00200000	/* Internal FLASH base address */
#define AT91SAM9XE_SRAM_BASE	0x00300000	/* Internal SRAM base address */

#define AT91SAM9G20_ROM_BASE	0x00100000	/* Internal ROM base address */
#define AT91SAM9G20_ROM_SIZE	SZ_32K		/* Internal ROM size (32Kb) */

#define AT91SAM9G20_SRAM0_BASE	0x00200000	/* Internal SRAM 0 base address */
#define AT91SAM9G20_SRAM0_SIZE	SZ_16K		/* Internal SRAM 0 size (16Kb) */
#define AT91SAM9G20_SRAM1_BASE	0x00300000	/* Internal SRAM 1 base address */
#define AT91SAM9G20_SRAM1_SIZE	SZ_16K		/* Internal SRAM 1 size (16Kb) */

#define AT91SAM9G20_UHP_BASE	0x00500000	/* USB Host controller */


/*Add by guowenxue 2011.08.01, copy from linux 2.6.22.1/include/asm/arch-at91/at91sam9260.h */

/*Part I*/
#define AT91C_US_RSTRX        (0x1 <<  2) 
#define AT91C_US_RSTTX        (0x1 <<  3) 
#define AT91C_US_RXEN         (0x1 <<  4) 
#define AT91C_US_RXDIS        (0x1 <<  5) 
#define AT91C_US_TXEN         (0x1 <<  6) 
#define AT91C_US_TXDIS        (0x1 <<  7) 
#define AT91C_US_RSTSTA       (0x1 <<  8) 

#define AT91C_US_PAR          (0x7 <<  9) 
#define     AT91C_US_PAR_EVEN                 (0x0 <<  9) 
#define     AT91C_US_PAR_ODD                  (0x1 <<  9) 
#define     AT91C_US_PAR_SPACE                (0x2 <<  9) 
#define     AT91C_US_PAR_MARK                 (0x3 <<  9) 
#define     AT91C_US_PAR_NONE                 (0x4 <<  9) 
#define     AT91C_US_PAR_MULTI_DROP           (0x6 <<  9) 
#define AT91C_US_CHMODE       (0x3 << 14) 
#define     AT91C_US_CHMODE_NORMAL               (0x0 << 14) 
#define     AT91C_US_CHMODE_AUTO                 (0x1 << 14) 
#define     AT91C_US_CHMODE_LOCAL                (0x2 << 14) 
#define     AT91C_US_CHMODE_REMOTE               (0x3 << 14) 

#define AT91C_US_RXRDY        (0x1 <<  0) 
#define AT91C_US_TXRDY        (0x1 <<  1) 
#define AT91C_US_ENDRX        (0x1 <<  3) 
#define AT91C_US_ENDTX        (0x1 <<  4) 
#define AT91C_US_OVRE         (0x1 <<  5) 
#define AT91C_US_FRAME        (0x1 <<  6) 
#define AT91C_US_PARE         (0x1 <<  7) 
#define AT91C_US_TXEMPTY      (0x1 <<  9) 
#define AT91C_US_TXBUFE       (0x1 << 11) 
#define AT91C_US_RXBUFF       (0x1 << 12) 
#define AT91C_US_COMM_TX      (0x1 << 30) 
#define AT91C_US_COMM_RX      (0x1 << 31) 
/*Part I End*/

/*Part II*/
#define AT91C_US_STTBRK       (0x1 <<  9) 
#define AT91C_US_STPBRK       (0x1 << 10) 
#define AT91C_US_STTTO        (0x1 << 11) 
#define AT91C_US_SENDA        (0x1 << 12) 
#define AT91C_US_RSTIT        (0x1 << 13) 
#define AT91C_US_RSTNACK      (0x1 << 14) 
#define AT91C_US_RETTO        (0x1 << 15) 
#define AT91C_US_DTREN        (0x1 << 16) 
#define AT91C_US_DTRDIS       (0x1 << 17) 
#define AT91C_US_RTSEN        (0x1 << 18) 
#define AT91C_US_RTSDIS       (0x1 << 19) 

#define AT91C_US_USMODE       (0xF <<  0) 
#define     AT91C_US_USMODE_NORMAL               (0x0) 
#define     AT91C_US_USMODE_RS485                (0x1) 
#define     AT91C_US_USMODE_HWHSH                (0x2) 
#define     AT91C_US_USMODE_MODEM                (0x3) 
#define     AT91C_US_USMODE_ISO7816_0            (0x4) 
#define     AT91C_US_USMODE_ISO7816_1            (0x6) 
#define     AT91C_US_USMODE_IRDA                 (0x8) 
#define     AT91C_US_USMODE_SWHSH                (0xC) 
#define AT91C_US_CLKS         (0x3 <<  4) 
#define     AT91C_US_CLKS_CLOCK                (0x0 <<  4) 
#define     AT91C_US_CLKS_FDIV1                (0x1 <<  4) 
#define     AT91C_US_CLKS_SLOW                 (0x2 <<  4) 
#define     AT91C_US_CLKS_EXT                  (0x3 <<  4) 
#define AT91C_US_CHRL         (0x3 <<  6) 
#define     AT91C_US_CHRL_5_BITS               (0x0 <<  6) 
#define     AT91C_US_CHRL_6_BITS               (0x1 <<  6) 
#define     AT91C_US_CHRL_7_BITS               (0x2 <<  6) 
#define     AT91C_US_CHRL_8_BITS               (0x3 <<  6) 
#define AT91C_US_SYNC         (0x1 <<  8) 
#define AT91C_US_NBSTOP       (0x3 << 12) 
#define     AT91C_US_NBSTOP_1_BIT                (0x0 << 12) 
#define     AT91C_US_NBSTOP_15_BIT               (0x1 << 12) 
#define     AT91C_US_NBSTOP_2_BIT                (0x2 << 12) 
#define AT91C_US_MSBF         (0x1 << 16) 
#define AT91C_US_MODE9        (0x1 << 17) 
#define AT91C_US_CKLO         (0x1 << 18) 
#define AT91C_US_OVER         (0x1 << 19) 
#define AT91C_US_INACK        (0x1 << 20) 
#define AT91C_US_DSNACK       (0x1 << 21) 
#define AT91C_US_MAX_ITER     (0x1 << 24) 
#define AT91C_US_FILTER       (0x1 << 28) 

#define AT91C_US_RXBRK        (0x1 <<  2) 
#define AT91C_US_TIMEOUT      (0x1 <<  8) 
#define AT91C_US_ITERATION    (0x1 << 10) 
#define AT91C_US_NACK         (0x1 << 13) 
#define AT91C_US_RIIC         (0x1 << 16) 
#define AT91C_US_DSRIC        (0x1 << 17) 
#define AT91C_US_DCDIC        (0x1 << 18) 
#define AT91C_US_CTSIC        (0x1 << 19) 

#define AT91C_US_RI           (0x1 << 20) 
#define AT91C_US_DSR          (0x1 << 21) 
#define AT91C_US_DCD          (0x1 << 22) 
#define AT91C_US_CTS          (0x1 << 23) 
/*Part II end*/

/*Part III for GTM900B driver*/
#define AT91C_ID_FIQ    ( 0) 
#define AT91C_ID_SYS    ( 1) 
#define AT91C_ID_PIOA   ( 2) 
#define AT91C_ID_PIOB   ( 3) 
#define AT91C_ID_PIOC   ( 4) 
#define AT91C_ID_ADC    ( 5) 
#define AT91C_ID_US0    ( 6) 
#define AT91C_ID_US1    ( 7) 
#define AT91C_ID_US2    ( 8) 
#define AT91C_ID_MCI    ( 9) 
#define AT91C_ID_UDP    (10) 
#define AT91C_ID_TWI    (11) 
#define AT91C_ID_SPI0   (12) 
#define AT91C_ID_SPI1   (13) 
#define AT91C_ID_SSC0   (14) 
#define AT91C_ID_TC0    (17) 
#define AT91C_ID_TC1    (18) 
#define AT91C_ID_TC2    (19) 
#define AT91C_ID_UHP    (20) 
#define AT91C_ID_EMAC   (21) 
#define AT91C_ID_HISI   (22) 
#define AT91C_ID_US3    (23) 
#define AT91C_ID_US4    (24) 
#define AT91C_ID_US5    (25) 
#define AT91C_ID_TC3    (26) 
#define AT91C_ID_TC4    (27) 
#define AT91C_ID_TC5    (28) 
#define AT91C_ID_IRQ0   (29) 
#define AT91C_ID_IRQ1   (30) 
#define AT91C_ID_IRQ2   (31) 
#define AT91C_ALL_INT   (0xFFFE7FFF) 
/*PART III End*/
/*Add end by guowenxue 2011.08.01*/

#endif
