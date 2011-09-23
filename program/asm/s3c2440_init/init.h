/*
 * =====================================================================================
 *
 *       Filename:  init.h
 *        Version:  1.0.0
 *         Author:  Guo Wenxue<Email: guowenxue@ghlsystems.com QQ:281143292>
 *      CopyRight:  2011 (C) Guo Wenxue
 *    Description:  Some Reigster address definition for init.S
 * =====================================================================================
 */

#define ELFIN_WATCHDOG_BASE     0x53000000

#define ELFIN_INTERRUPT_BASE    0x4a000000
#define SRCPND_OFFSET           0x00
#define INTMOD_OFFSET           0x04
#define INTMSK_OFFSET           0x08
#define PRIORITY_OFFSET         0x0c
#define INTPND_OFFSET           0x10
#define INTOFFSET_OFFSET        0x14
#define SUBSRCPND_OFFSET        0x18
#define INTSUBMSK_OFFSET        0x1c

#define ELFIN_CLOCK_POWER_BASE  0x4c000000
#define LOCKTIME_OFFSET         0x00
#define MPLLCON_OFFSET          0x04
#define UPLLCON_OFFSET          0x08
#define CLKCON_OFFSET           0x0c
#define CLKSLOW_OFFSET          0x10
#define CLKDIVN_OFFSET          0x14
#define CAMDIVN_OFFSET          0x18

#define BWSCON  		0x48000000

#define MDIV_405                0x7f << 12
#define PSDIV_405               0x21

/*  BWSCON */
#define DW8         (0x0)
#define DW16            (0x1)
#define DW32            (0x2)
#define WAIT            (0x1<<2)
#define UBLB            (0x1<<3)

#define B1_BWSCON       (DW16)
#define B2_BWSCON       (DW16)
#define B3_BWSCON       (DW16 + WAIT + UBLB)
#define B4_BWSCON       (DW16)
#define B5_BWSCON       (DW16)
#define B6_BWSCON       (DW32)
#define B7_BWSCON       (DW32)

#define B0_Tacs         0x0
#define B0_Tcos         0x0
#define B0_Tacc         0x7
#define B0_Tcoh         0x0
#define B0_Tah          0x0
#define B0_Tacp         0x0
#define B0_PMC          0x0

#define B1_Tacs         0x0
#define B1_Tcos         0x0
#define B1_Tacc         0x7
#define B1_Tcoh         0x0
#define B1_Tah          0x0
#define B1_Tacp         0x0
#define B1_PMC          0x0 

#define B2_Tacs         0x0
#define B2_Tcos         0x0
#define B2_Tacc         0x7
#define B2_Tcoh         0x0
#define B2_Tah          0x0
#define B2_Tacp         0x0
#define B2_PMC          0x0

#define B3_Tacs         0xc
#define B3_Tcos         0x7
#define B3_Tacc         0xf
#define B3_Tcoh         0x1
#define B3_Tah          0x0
#define B3_Tacp         0x0
#define B3_PMC          0x0

#define B4_Tacs         0x0
#define B4_Tcos         0x0
#define B4_Tacc         0x7
#define B4_Tcoh         0x0
#define B4_Tah          0x0
#define B4_Tacp         0x0
#define B4_PMC          0x0

#define B5_Tacs         0xc
#define B5_Tcos         0x7
#define B5_Tacc         0xf
#define B5_Tcoh         0x1
#define B5_Tah          0x0
#define B5_Tacp         0x0
#define B5_PMC          0x0

#define B6_MT           0x3 /*  SDRAM */
#define B6_Trcd         0x1
#define B6_SCAN         0x1 /*  9bit */

#define B7_MT           0x3 /*  SDRAM */
#define B7_Trcd         0x1 /*  3clk */
#define B7_SCAN         0x1 /*  9bit */

/*  REFRESH parameter */
#define REFEN           0x1 /*  Refresh enable */
#define TREFMD          0x0 /*  CBR(CAS before RAS)/Auto refresh */
#define Trc             0x3 /*  7clk */
#define Tchr            0x2 /*  3clk */

#if defined(CONFIG_S3C2440)
#define Trp             0x2 /*  4clk */
#define REFCNT          1012
#else
#define Trp             0x0 /*  2clk */
#define REFCNT          0x0459
#endif 
