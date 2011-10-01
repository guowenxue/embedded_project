/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the AT91SAM9260EK & AT91SAM9G20EK boards.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_AT91_LEGACY

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000	/* 18.432 MHz crystal */
#define CONFIG_SYS_HZ		1000

#define CONFIG_ARM926EJS	1	/* This is an ARM926EJS Core	*/

#ifdef CONFIG_AT91SAM9G20EK
#define CONFIG_AT91SAM9G20	1	/* It's an Atmel AT91SAM9G20 SoC*/
#define CONFIG_CPU        "sam9g20"
#else
#define CONFIG_AT91SAM9260	1	/* It's an Atmel AT91SAM9260 SoC*/
#define CONFIG_CPU        "sam9260"
#endif

#define CONFIG_ARCH_CPU_INIT
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SKIP_RELOCATE_UBOOT

/*
 * Hardware drivers
 */
#define CONFIG_AT91_GPIO	1
#define CONFIG_ATMEL_USART	1
#undef CONFIG_USART0
#undef CONFIG_USART1
#undef CONFIG_USART2
#define CONFIG_USART3		1	/* USART 3 is DBGU */

/* LED */
#define CONFIG_AT91_LED
//#define	CONFIG_RED_LED		AT91_PIN_PA9	/* this is the power led */
//#define	CONFIG_GREEN_LED	AT91_PIN_PA6	/* this is the user led */
#define CONFIG_RUN_LED      AT91_PIN_PA26   /* Add by guowenxue, 2011.08.11*/
#define CONFIG_BEEP         AT91_PIN_PC2    /*Add by guowenxue, 2011.08.11*/
#define CONFIG_RESTORE_KEY  AT91_PIN_PB20
#define CONFIG_BEEP_ALARM   
#define CONFIG_SILENT_CONSOLE          /*Add by guowenxue*/ 

#define CONFIG_BOOTDELAY	1

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE	1
#define CONFIG_BOOTP_BOOTPATH		1
#define CONFIG_BOOTP_GATEWAY		1
#define CONFIG_BOOTP_HOSTNAME		1

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_SOURCE

#define CONFIG_CMD_PING		1
#define CONFIG_CMD_DHCP		1
#define CONFIG_CMD_NAND		1
//#define CONFIG_CMD_USB		1  /*Comment by guowenxue*/

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			0x20000000
#define PHYS_SDRAM_SIZE			0x04000000	/* 64 megs */

/* DataFlash */
#define CONFIG_ATMEL_DATAFLASH_SPI
#define CONFIG_HAS_DATAFLASH		1
#define CONFIG_SYS_SPI_WRITE_TOUT		(5*CONFIG_SYS_HZ)
#define CONFIG_SYS_MAX_DATAFLASH_BANKS		2
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* CS0 */
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS1	0xD0000000	/* CS1 */
#define AT91_SPI_CLK			15000000

#ifdef CONFIG_AT91SAM9G20EK
#define DATAFLASH_TCSS			(0x22 << 16)
#else
#define DATAFLASH_TCSS			(0x1a << 16)
#endif
#define DATAFLASH_TCHS			(0x1 << 24)

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			0x40000000
#define CONFIG_SYS_NAND_DBW_8			1
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PC13

#endif

/* NOR flash - no real flash on this board */
#define CONFIG_SYS_NO_FLASH			1

/*Add by guowenxue to support YAFFS2 file system*/
//#define CONFIG_CMD_NAND_YAFFS2          1
//#define CONFIG_CMD_NAND_YAFFS2_SKIPFB   1
/*Add by guowenxue end*/

/* Ethernet */
#define CONFIG_OVERWRITE_ETHADDR_ONCE   1  /*Add by guowenxue to support Modify MAC address*/
#define CONFIG_MACB			1
#undef  CONFIG_RMII			/*undef by guowenxue*/
#define CONFIG_NET_MULTI		1
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R		1
/*Add DM9000 support by guowenxue*/
#define CONFIG_DRIVER_DM9000        1          /* Enable DM9000 define and disable DM9161*/
#define CONFIG_DM9000_BASE      0x50000000     /* CS4    */
#define DM9000_IO           CONFIG_DM9000_BASE
#define DM9000_DATA         (CONFIG_DM9000_BASE + 4)     /*  A2-->CMD*/
#define CONFIG_DM9000_USE_16BIT     1
#define CONFIG_DM9000_NO_SROM       1

/* USB */
#if 0 /*Comment by guowenxue, don't enable USB interface*/
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW		1
#define CONFIG_DOS_PARTITION		1
#define CONFIG_SYS_USB_OHCI_CPU_INIT		1
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00500000	/* AT91SAM9260_UHP_BASE */
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9260"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_USB_STORAGE		1
#define CONFIG_CMD_FAT			1
#endif

#define CONFIG_SYS_LOAD_ADDR			0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START		PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END			0x23e00000

#ifdef CONFIG_SYS_USE_DATAFLASH_CS0

/* bootstrap + u-boot + env + linux in dataflash on CS0 */
#define CONFIG_ENV_IS_IN_DATAFLASH	1
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0 + 0x8400)
#define CONFIG_ENV_OFFSET		0x4200
#define CONFIG_ENV_ADDR		(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0 + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		0x4200
#define CONFIG_BOOTCOMMAND	"nand read 20000000 100000 500000;bootm 20000000"
#define CONFIG_BOOTARGS		"noinitrd root=/dev/mtdblock4 rootfstype=yaffs2 "" \
                            "init=/linuxrc console=ttyS0,115200 mem=64M loglevel=7 lpj=495616"

#elif CONFIG_SYS_USE_DATAFLASH_CS1

/* bootstrap + u-boot + env + linux in dataflash on CS1 */
#define CONFIG_ENV_IS_IN_DATAFLASH	1
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS1 + 0x8400)
#define CONFIG_ENV_OFFSET		0x4000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS1 + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		0x4400
#define CONFIG_BOOTCOMMAND	"nand read 20000000 a0000 0x500000;nand read 21100000 5a0000 500000;bootm 20000000"
#define CONFIG_BOOTARGS		"console=/dev/ttyS0,115200 initrd=0x21100040,5M root=/dev/ram0 rw time" 
#define CONFIG_ETHADDR      CONFIG_MAC_ADDR
#define CONFIG_IPADDR       192.168.200.120
#define CONFIG_SERVERIP     192.168.200.15

#define CONFIG_KERNEL_NAND_ADDR           0xa0000
#define CONFIG_ROOTFS_NAND_ADDR           0x5a0000

#define CONFIG_BURNKERNEL   "tftp 20000000 uImage-$(cpu).gz;" \
                            "nand erase 000a0000 500000;nand write 20000000 000a0000 $(filesize)"
#define CONFIG_BURNFS       "tftp 21100000 ramdisk-$(cpu).gz;nand erase 5a0000 500000;" \
                            "nand write 21100000 5a0000 $(filesize)"
#define CONFIG_BURNBL       "tftp 20000000 u-boot-$(cpu).bin;protect off 0xD0008400 0xD003FFFF;" \
                            "erase 0xD0008400 0xD003FFFF;cp.b 20000000 d0008400 $(filesize)"
#define CONFIG_TFTPBOOT     "tftp 20000000 uImage-$(cpu).gz;tftp 21100000 ramdisk-$(cpu).gz;bootm 20000000"

#else /* CONFIG_SYS_USE_NANDFLASH */

/* bootstrap + u-boot + env + linux in nandflash */
#define CONFIG_ENV_IS_IN_NAND	1
#define CONFIG_ENV_OFFSET		0x60000
#define CONFIG_ENV_OFFSET_REDUND	0x80000
#define CONFIG_ENV_SIZE		0x20000		/* 1 sector = 128 kB */
#define CONFIG_BOOTCOMMAND	"nand read 0x22000000 0xA0000 0x200000; bootm"
#define CONFIG_BOOTARGS		"console=ttyS0,115200 "			\
				"root=/dev/mtdblock5 "			\
				"mtdparts=atmel_nand:128k(bootstrap)ro,"	\
				"256k(uboot)ro,128k(env1)ro,"		\
				"128k(env2)ro,2M(linux),-(root) "	\
				"rw rootfstype=jffs2"

#endif

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{115200 , 19200, 38400, 57600, 9600 }

#define CONFIG_SYS_PROMPT		"U-Boot> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP		1
#define CONFIG_CMDLINE_EDITING	1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		ROUND(3 * CONFIG_ENV_SIZE + 128*1024, 0x1000)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* 128 bytes for initial data */

#define CONFIG_STACKSIZE	(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif
