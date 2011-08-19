/** \file
 *  \brief Structure defines for low level interface.
 *  
 *  Includes TWI Structures
 */

#ifndef LL_PORT_H
#define LL_PORT_H

#include "CM_LIB.H"
#include "plat_ioctl.h"

/** Low Level TWI Defines */
#define LL_START_TRIES 10       ///< Defines # of Start attempts to send command
#define LL_PWRON_CLKS  15       ///< Number of clocks to reset CM on power up
#define LL_ACK_TRIES    8       ///< Number of times to try to get ACK on a write

//#define EEPROM_DEBUG
#ifdef EEPROM_DEBUG
#define eep_print(format,args...) printf(format, ##args)
#else
#define eep_print(format,args...) do{} while(0);
#endif

#if (defined CPU_AT91SAM9260)
/* L300,L350,N300,L200_ARM9 use AT91SAM9260(ARM9) cpu, it will provider GPIO simulator
 * I2C driver in Linux kernel. Follow is the ioctl() cmd defined in the driver.
 */
#if 0  /*This is old driver ioctl command, from driver 2.0.0, it defines in plat_ioctl.h*/
#define LL_POWERON  0x8080
#define LL_DATALOW  0x8081
#define LL_STOP     0x8082
#define LL_ACKNAK   0x8083
#define LL_START    0x8084
#define LL_WRITE    0x8085
#define LL_READ     0x8086
#define LL_POWEROFF 0x8087
#endif

#elif (defined CPU_W90P710)
/* L100,L200,L200_OPT use W90P170(ARM7) cpu, it will use GPIO to simulator I2C in
 * user space in file i2c_w90p710.c, follow is the ll_port.c depend on function.
 */
void i2c_init(void);
void i2c_Start(void);
void i2c_Stop(void);
void i2c_AckNak(uchar ucAck);
void i2c_PowerOn(void);
void i2c_PowerOff(void);
void i2c_WaitClock(uchar loop);
void i2c_ClockCycles(uchar ucCount);
uchar i2c_Write(uchar ucData);
uchar i2c_Read(void);
#endif

/*Extern declair function*/
int device_eep_init(char *dev_name);
void device_eep_term(void);
void ll_Start(void);
void ll_Stop(void);
void ll_AckNak(uchar ucAck);
void ll_PowerOn(void);
void ll_PowerOff(void);
void ll_WaitClock(uchar loop);
uchar ll_Write(uchar ucData);
uchar ll_Read(void);
uchar ll_SendCommand(puchar pucInsBuf);
uchar ll_ReceiveData(puchar pucRecBuf, uchar ucLen);
uchar ll_SendData(puchar pucSendBuf, uchar ucLen);

#endif                          // LL_PORT_H
