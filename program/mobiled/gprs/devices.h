#ifndef __DEVICES_H
#define __DEVICES_H

#ifndef DEV_LEN
#define DEV_LEN               64    /*Device Name Length */
#endif

/*Buzzer & LED*/
#define DEV_BEEP              "/dev/beep"
#define DEV_LED               "/dev/led"

/*EEPROM device*/
#define DEV_EEPROM_CARD       "/dev/eeprom"
#if (defined PLAT_N300)         /*Only N300 get EEPROM */
#define DEV_EEPROM_BACK       "/dev/bp-eeprom"
#else
#define DEV_EEPROM_BACK       "/dev/null"   /*Not support */
#endif

/*GPRS, Serial port, and PSTN*/
#define GPRS_CTRLPORT         "/dev/gprs"

#if (defined PLAT_L100 || defined PLAT_L200)
#define GPRS_COMPORT         "/dev/ttyS1"
#define GPRS3G_COMPORT       "/dev/ttyUC864E0"
#elif( defined PLAT_L300 || defined PLAT_L350)
#define GPRS_COMPORT         "/dev/ttyS2"
#define GPRS3G_COMPORT       "/dev/ttyUSB0"
#elif (defined PLAT_N300)
#define GPRS_COMPORT         "/dev/null"    /*Not support */
#define GPRS3G_COMPORT       "/dev/ttyUSB0"
#endif

#define CONSOLE_COMPORT       "/dev/ttyS0"

#if (defined PLAT_L200 || defined PLAT_L100 || defined PLAT_L300 || defined PLAT_L350)
#define MAX_PSTN_CHANNELS     1
#define PSTN1_CTL             "/dev/modem"
#define PSTN1_TTY             "/dev/ttyS1"
#elif (defined PLAT_N300)
#define MAX_PSTN_CHANNELS     8 /*0 */

/***********************************
   N300 PSTN  Matrix
----+---------------------+----
|   |  ttyS1   |  ttyS2   |   |
| 1 |  modem1  |  modem5  | 5 |
|   |   U13    |   U32    |   |
|---|---------------------|---|
|   |  ttyS3   |  ttyS0   |   |
| 2 |  modem2  |  modem6  | 6 |
|   |   U15    |   U30    |   |
|---|---------------------|---|
|   |  ttyS5   |  ttyS6   |   |
| 3 |  modem3  |  modem7  | 7 |
|   |   U14    |   U33    |   |
|---|---------------------|---|
|   |  ttySPI0 |  ttySPI1 |   |
| 4 |  modem4  |  modem8  | 8 |
|   |   U16    |   U31    |   |
|---|---------------------|---|
***********************************/

#define PSTN1_CTL             "/dev/modem1"
#define PSTN1_TTY             "/dev/ttyS1"

#define PSTN2_CTL             "/dev/modem2"
#define PSTN2_TTY             "/dev/ttyS3"

#define PSTN3_CTL             "/dev/modem3"
#define PSTN3_TTY             "/dev/ttyS5"

#define PSTN4_CTL             "/dev/modem4"
#define PSTN4_TTY             "/dev/ttySPI0"

#define PSTN5_CTL             "/dev/modem5"
#define PSTN5_TTY             "/dev/ttyS2"

#define PSTN6_CTL             "/dev/modem6"
#define PSTN6_TTY             "/dev/ttyS0"

#define PSTN7_CTL             "/dev/modem7"
#define PSTN7_TTY             "/dev/ttyS6"

#define PSTN8_CTL             "/dev/modem8"
#define PSTN8_TTY             "/dev/ttySPI1"
#endif

typedef struct __PSTN_CHANNEL
{
    char ctl[DEV_LEN];          /*PSTN control port device name */
    char tty[DEV_LEN];          /*PSTN data port device name */
} PSTN_CHANNEL;

int open_led_dev(void);
void turn_led_on(void);
void turn_led_off(void);
void close_led_dev(void);

int open_beep_dev(void);
void start_beep(int times);
void close_beep_dev(void);

void beep_led_blink(int times);

unsigned char pstn_test(int index);

#endif
