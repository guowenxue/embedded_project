/*********************************************************************************
 *  Copyright(c)  2011, Guo Wenxue <guowenxue@gmail.com>
 *  All ringhts reserved.
 *
 *     Filename:  plat_l2.h
 *  Description:  L2(Based on AT91SAM9260 MCU) GPIO port definition
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-08-10
 *            Author: guowenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 *
 ********************************************************************************/

#ifndef __PLAT_L2_H
#define __PLAT_L2_H

#include "plat_ioctl.h"
#define UNDEFINED 0xFFFFFFFF

/*=========================================================================
 *             For L2(ARM9)
 *=========================================================================*/
/*Pin definition For GPRS/3G Driver*/
#define GPRS_VBUS_CTRL_PIN          AT91_PIN_PC9  /* GPRS USB VBUS control pin */
#define GPRS_POWER_MON_PIN          UNDEFINED     /*NC*/
#define GPRS_CHK_SIM2_PIN           UNDEFINED     /*NC*/
#define GPRS_RTS_PIN                UNDEFINED     /*NC*/
#define GPRS_ON_PIN                 AT91_PIN_PB30 /* GPRS power on/off switch pin */
#define GPRS_38V_ON_PIN             AT91_PIN_PA10 /* GPRS module 3.8V power supply Pin, 1->On 0->off */
#define GPRS_RESET_PIN              AT91_PIN_PA11 /* GPRS Reset Pin*/
#define GPRS_DTR_PIN                AT91_PIN_PA22 /*Set GPIO Highlevel*/
#define GPRS_RI_PIN                 AT91_PIN_PA25
#define GPRS_TXD_PIN                AT91_PIN_PB6  
#define GPRS_RXD_PIN                AT91_PIN_PB7
#define GPRS_SELECT_SIM_PIN         AT91_PIN_PC1
#define GPRS_CHK_SIM1_PIN           AT91_PIN_PC6

#define GPRS_MODEL_CHK_PIN0         AT91_PIN_PB31
#define GPRS_MODEL_CHK_PIN1         AT91_PIN_PB12
#define GPRS_MODEL_CHK_PIN2         AT91_PIN_PB13

/*Pin definition For LED Driver*/
#define LED_COUNT                   5
#define LED_D1_RUN                  AT91_PIN_PA26
#define LED_D2_0                    AT91_PIN_PC3
#define LED_D3_1                    AT91_PIN_PC7
#define LED_D4_2                    AT91_PIN_PC12
#define LED_D5_3                    AT91_PIN_PB21
//#define LED_D6_4                    AT91_PIN_PB31

/*Pin definition For Beep Driver*/
#define BEEP_PIN                    AT91_PIN_PC2

/*Pin definition For EEPROM Driver*/
#define TWI_SCLK                    AT91_PIN_PA24
#define TWI_SDAT                    AT91_PIN_PA23

/*Pin definition For Modem driver*/
#define MODEM_PIN_RESET             AT91_PIN_PB20   /*Low level take affect */
#define MODEM_PIN_RXD               AT91_PIN_PB5
#define MODEM_PIN_TXD               AT91_PIN_PB4
#define MODEM_PIN_CTS               AT91_PIN_PB27
#define MODEM_PIN_RTS               AT91_PIN_PB26
#define MODEM_PIN_DTR               AT91_PIN_PB24
#define MODEM_PIN_DCD               AT91_PIN_PB23
#define MODEM_PIN_RI                AT91_PIN_PB25

/*Pin definition For RS485*/
#define RS485_1_CRTL                AT91_PIN_PB3
#define RS485_1_TXD                 AT91_PIN_PB8
#define RS485_1_RXD                 AT91_PIN_PB9
#define RS485_2_CRTL                AT91_PIN_PA29
#define RS485_2_TXD                 AT91_PIN_PB31
#define RS485_2_RXD                 AT91_PIN_PB30

/* Pin definition for SAM card driver */
#define SAM_SCK                     AT91_PIN_PC0    // closk
#define SAM_RST                     AT91_PIN_PB11   // reset
#define SAM_DATA                    AT91_PIN_PB10   // I/O
#define SAM_CHK                     AT91_PIN_PA27   // Input, detect SAM card
#define SAM_M0                      AT91_PIN_PB16
#define SAM_M1                      AT91_PIN_PB17
#define SAM_M2                      AT91_PIN_PB18
#define FAKE_SAM_M0                 AT91_PIN_PA28
#define FAKE_SAM_M1                 AT91_PIN_PA29
#define FAKE_SAM_M2                 AT91_PIN_PA30

/* Pin definition for restore key driver */
#define RESTORE_KEY                   AT91_PIN_PB20
#define RESTORE_KEY_IRQ               50

/* Function switch */
#define FUNC_SWITCH_SUPPORT         1
#define FUNC_SWITCH_PIN0            AT91_PIN_PB3
#define FUNC_SWITCH_PIN1            AT91_PIN_PB2
#define FUNC_SWITCH_PIN2            AT91_PIN_PB1
#define FUNC_SWITCH_PIN3            AT91_PIN_PB0

#endif                          /*End __PLAT_L2_H */
