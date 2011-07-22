/********************************************************************************************
 *        File:  start.S - Startup Code for ARM920 CPU-core
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  Configure file for bootstrap, just like u-boot-1.3.4/include/config/s3c2440.h
 *   ChangeLog:  1, Release initial version on "Tue Jul 12 16:43:18 CST 2011"
 *
 *******************************************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef BOARD_SMDK2440
#include <s3c2440.h>
#define CONFIG_S3C2440
#endif

#define MDIV_405                    0x7f << 12
#define PSDIV_405                   0x21

#define CONFIG_LED_DEBUG            1

#define CONFIG_SYS_MALLOC_LEN       (1024*1024)
#define CONFIG_SYS_GBL_DATA_SIZE    128
#define CONFIG_STACKSIZE            0x40000

#define CFG_PBSIZE                  384 /* Print Buffer Size */

#define CFG_MAX_NAND_DEVICE         1
#define NAND_MAX_CHIPS              1
#define CFG_NAND_BASE               0x4E000000

#define BOOTLOADER_ENV_ADDR       0x20000   /*Bootloader enviroment start address*/
#define LAUNCHER_NAND_ADDR        0x40000   /*Laucher save in Nandflash address*/

#endif
