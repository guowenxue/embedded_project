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

#define S3C_WATCHDOG_BASE       0x53000000
#define S3C_INTERRUPT_BASE      0x4a000000
#define SRCPND_OFFSET           0x00
#define INTMOD_OFFSET           0x04
#define INTMSK_OFFSET           0x08
#define PRIORITY_OFFSET         0x0c
#define INTPND_OFFSET           0x10
#define INTOFFSET_OFFSET        0x14
#define SUBSRCPND_OFFSET        0x18
#define INTSUBMSK_OFFSET        0x1c

/*GPIO for LED register*/
#define GPACON                  0x56000000
#define GPADAT                  0x56000004
#define LED1                    1  /*On S3C2416 board, LED1 use GPA1*/
#define LED2                    2  /*On S3C2416 board, LED2 use GPA2*/

#define S3C_CLOCK_POWER_BASE    0x4c000000
#define LOCKCON0_OFFSET         0x00
#define LOCKCON1_OFFSET         0x04
#define MPLLCON_OFFSET          0x10
#define EPLLCON_OFFSET          0x18
#define CLKSRCCON_OFFSET        0x20
#define CLKDIV0CON_OFFSET       0x24

#define S3C_MEMCTL_BASE         0x48000000
#define BANKCFG_OFFSET          0x00
#define BANKCON1_OFFSET         0x04
#define BANKCON2_OFFSET         0x08
#define BANKCON3_OFFSET         0x0c
#define REFRESH_OFFSET          0x10
#define TIMEOUT_OFFSET          0x14

/*Memory Controller Register Initiliaze Value*/
#define INIT_NORMAL 0x0
#define INIT_PALL   0x1
#define INIT_MRS    0x2
#define INIT_EMRS   0x3
#define INIT_MASK   0x3

#define CFG_BANK_CFG_VAL    0x0004920d
#define CFG_BANK_CON1_VAL   0x04000040
#define CFG_BANK_CFG_VAL_ALT    0x00048904
#define CFG_BANK_CON1_VAL_ALT   0x04000040
#define CFG_BANK_CFG_VAL_DDR2   0x00049253
#define CFG_BANK_CON1_VAL_DDR2  0x44000040

#define CFG_BANK_CON2_VAL   0x0057003a
#define CFG_BANK_CON2_VAL_DDR2  0x005D0035
#define CFG_BANK_CON3_VAL   0x80000030
#define CFG_BANK_REFRESH_VAL    0x00000313

/* Clock And Power Controller Register Initiliaze Value */
//#define CONFIG_CLK_534_133_66
#define CONFIG_CLK_400_133_66
//#define CONFIG_CLK_267_133_66

#if defined(CONFIG_CLK_400_133_66)/* Output CLK 800MHz 2:4:8*/
#define Startup_MDIV        400
#define Startup_PDIV        3
#define Startup_SDIV        1
#define Startup_EPLLSTOP    0
#define Startup_ARMCLKdiv   1
#define Startup_PREdiv      0x2
#define Startup_HCLKdiv     0x1
#define Startup_PCLKdiv     1
#elif defined (CONFIG_CLK_534_133_66)
#define Startup_MDIV        267
#define Startup_PDIV        3
#define Startup_SDIV        1
#define Startup_EPLLSTOP    0
#define Startup_ARMCLKdiv   0
#define Startup_PREdiv      0x1
#define Startup_HCLKdiv     0x1
#define Startup_PCLKdiv     1
#elif defined (CONFIG_CLK_267_133_66)
#define Startup_MDIV        267
#define Startup_PDIV        3
#define Startup_SDIV        1
#define Startup_EPLLSTOP    0
#define Startup_ARMCLKdiv   1
#define Startup_PREdiv      0x1
#define Startup_HCLKdiv     0x1
#define Startup_PCLKdiv     1
#else
# error Must define CONFIG_CLK_534_133_66 or CONFIG_CLK_400_133_66
#endif

#define CLK_DIV_VAL ((Startup_ARMCLKdiv<<9)|(Startup_PREdiv<<4)|(Startup_PCLKdiv<<2)|(Startup_HCLKdiv)|(1<<3))
#define MPLL_VAL    ((Startup_EPLLSTOP<<24)|(Startup_MDIV<<14)|(Startup_PDIV<<5)|(Startup_SDIV))
#define EPLL_VAL    (32<<16)|(1<<8)|(2<<0)



