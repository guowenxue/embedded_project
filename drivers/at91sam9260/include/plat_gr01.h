/*********************************************************************************
 *  Copyright(c)  2011,  GuoWenxue<guowenxue@gmail.com>.
 *  All ringhts reserved.
 *
 *     Filename:  plat_gr01.h
 *  Description:  GR01 based on AT91SAM9260 MCU) GPIO port definition
 *
 *  Hardware Information:
 *          CPU:  AT91SAM9260 (ARM926EJS,200MHz)      
 *        SDRAM:  H57V561620A (2*32MB)
 *    NandFlash:  K9F2G08U0M  (256MB)
 *SPI DataFlash:  AT45DB321D  (4MB)
 *       EEPROM:  AT88SC0104C (Security EEPROM)
 *      3G/GPRS:  Gospell G600
 *    Interface:  RS232, 1*USB Host, 1*USB Device, 2* Ethernt(DM9000+DM9161), 
 *                Don't support SD card & RS485 & JTAG
 *         MISC:  Beep, LED, RTC, 
 *
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-05-02
 *            Author: guowenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 *
 ********************************************************************************/

#ifndef __PLAT_GR01_H
#define __PLAT_GR01_H

#include "plat_ioctl.h"

/*=========================================================================
 *             For GR01(ARM9)
 *=========================================================================*/
/*Pin definition For GPRS/3G Driver*/

#define GPRS_ON_PIN                 AT91_PIN_PA10 
#define GPRS_RESET_PIN              AT91_PIN_PA11
#define GPRS_DSR_PIN                AT91_PIN_PA25 
#define GPRS_DTR_PIN                AT91_PIN_PA27 
#define GPRS_DCD_PIN                AT91_PIN_PA28 
#define GPRS_TXD_PIN                AT91_PIN_PB6
#define GPRS_RXD_PIN                AT91_PIN_PB7
#define GPRS_RTS_PIN                AT91_PIN_PB28
#define GPRS_CTS_PIN                AT91_PIN_PB29 
#define GPRS_RI_PIN                 AT91_PIN_PB30 

#define GPRS_CHK_SIM1_PIN           AT91_PIN_PC6  /*Only support 1 SIM slot */
#define GPRS_VBUS_CTRL_PIN          AT91_PIN_PC9  /*For 3G GPRS module*/

/* For DTR/FTU extend use */
#define SAMPLE_PORT0                AT91_PIN_PB12
#define SAMPLE_PORT1                AT91_PIN_PB13
#define SAMPLE_PORT2                AT91_PIN_PB16
#define SAMPLE_PORT3                AT91_PIN_PB17
#define SAMPLE_PORT4                AT91_PIN_PB18
#define SD_PWR                      AT91_PIN_PB19
#define SAMPLE_PORT5                AT91_PIN_PB20
#define SAMPLE_PORT6                AT91_PIN_PB21
#define SAMPLE_PORT7                AT91_PIN_PB22
#define SAMPLE_PORT8                AT91_PIN_PB23
#define SAMPLE_PORT9                AT91_PIN_PB24

/* LED definintion on PCB: 
 * LED2->Power_LED, LED3->RUN_LED, LED4->GPRS_LED, LED5->ETH1_LED(DM9161), 
 * LED6->ETH2_LED(DM9000),  LED7->LED0,    LED8->LED1 */
#define LED_LENGHT                  3
#define LED_D1_RUN                  AT91_PIN_PA26
#define LED0_PIN                    AT91_PIN_PB0
#define LED1_PIN                    AT91_PIN_PB1

/* GR01 Beep Pin definition */
#define BEEP_PIN                    AT91_PIN_PC2

/* GR01 AT88SC0104C EEPROM Pin definition */
#define TWI_SCLK                    AT91_PIN_PA24
#define TWI_SDAT                    AT91_PIN_PA23

#endif                          /*End __PLAT_GR01_H */
