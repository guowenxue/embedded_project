/** \file
 *  \brief CryptoMemory Hardware Interface Functions - TWI-like bus.
 *
 *  The first version of the CryptoMemory Synchronous Interface is not
 *  a true TWI interface.  Reading the CryptoMemory is not done with
 *  the read bit set in the address, following a restart.
 *  Instead the read is done immediately after setting the address with
 *  a write instruction.
 */
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include "ll_port.h"

int s_fd = -1;

/** \open eeprom driver */
int device_eep_init(char *dev_name)
{
#if (defined CPU_AT91SAM9260)
    eep_print("Open EEPROM device %s\n", dev_name);
    s_fd = open("/dev/eeprom", O_RDWR);
    if (s_fd < 0)
    {
        return -1;
    }
#elif (defined CPU_W90P710)
    eep_print("Initialize w90p710 eeprom\n");
    i2c_init();
#endif

    return 0;
}

 /** \close eeprom driver */
void device_eep_term(void)
{
#if (defined CPU_AT91SAM9260)
    eep_print("Term EEPROM device\n");
    close(s_fd);
#endif
}

/** \brief Calls ll_ClockCycle() ucCount times */
void ll_ClockCycles(uchar ucCount)
{
#if (defined CPU_AT91SAM9260)
    unsigned char iodat = ucCount;
    ioctl(s_fd, LL_DATALOW, &iodat);
#elif (defined CPU_W90P710)
    i2c_ClockCycles(ucCount);
#endif
}

/** \brief Sends a start sequence */
void ll_Start(void)
{
#if (defined CPU_AT91SAM9260)
    unsigned char iodat;
    ioctl(s_fd, LL_START, &iodat);
#elif (defined CPU_W90P710)
    i2c_Start();
#endif
}

/** \brief  Sends a stop sequence */
void ll_Stop(void)
{
#if (defined CPU_AT91SAM9260)
    unsigned char iodat;
    ioctl(s_fd, LL_STOP, &iodat);
#elif (defined CPU_W90P710)
    i2c_Stop();
#endif
}

/** \brief Sends an ACK or NAK to the device (after a read). 
 * 
 *  \param ucAck - if ::TRUE means an ACK will be sent, otherwise
 *  NACK sent.
 */
void ll_AckNak(uchar ucAck)
{
#if (defined CPU_AT91SAM9260)
    unsigned char iodat;
    ioctl(s_fd, LL_ACKNAK, &iodat);
#elif (defined CPU_W90P710)
    i2c_AckNak(ucAck);
#endif
}

/** \brief Power On chip sequencing.  Clocks the chip ::LL_PWRON_CLKS times. 
*/
void ll_PowerOn(void)
{
#if (defined CPU_AT91SAM9260)
    unsigned char iodat;
    ioctl(s_fd, LL_POWERON, &iodat);
#elif (defined CPU_W90P710)
    i2c_PowerOn();
#endif
}

void ll_PowerOff(void)
{
#if (defined CPU_AT91SAM9260)
    unsigned char iodat = 0;
    ioctl(s_fd, LL_POWEROFF, &iodat);
#elif (defined CPU_W90P710)
    i2c_PowerOff();
#endif
}

/** \brief  Write a byte on the TWI-like bus.
*
* \return 0 if write success, else 'n' (the number of attempts) 
*  to get ACK that failed (maximum is ::LL_ACK_TRIES).
*/
uchar ll_Write(uchar ucData)
{
    unsigned char iodat;
#if (defined CPU_AT91SAM9260)
    iodat = ucData;
    if (ioctl(s_fd, LL_WRITE, &iodat) < 0)
        return 0xFF;
#elif (defined CPU_W90P710)
    iodat = i2c_Write(ucData);
#endif
    return iodat;
}

/** \brief Read a byte from device, MSB first, no ACK written yet*/
uchar ll_Read(void)
{
    unsigned char iodat = 0;
#if (defined CPU_AT91SAM9260)
    if (ioctl(s_fd, LL_READ, &iodat) < 0)
    {
        return 0xFF;
    }
#elif (defined CPU_W90P710)
    iodat = i2c_Read();
#endif
    return iodat;
}

/** \brief Send a command (usually 4 bytes) over the bus.
 * 
 * \param pucInsBuf is pointer to a buffer containing numBytes to send
 * \param numBytes is the number of bytes to send
 *
 * \return ::SUCCESS or ::FAIL_CMDSEND (if ll_Write() fails)
 */
uchar ll_SendCommand(puchar pucInsBuf)
{
    uchar i = LL_START_TRIES;
    while (i)
    {
        ll_Start();
        if (ll_Write(pucInsBuf[0]) == 0)
            break;
        if (--i == 0)
        {
            eep_print("%s() send first byte can not get ACK.\n", __FUNCTION__);
            return FAIL_CMDSTART;
        }
    }

    /*Send commmand buffer always is 4 bytes */
    for (i = 1; i < 4; i++)
    {
        if (ll_Write(pucInsBuf[i]) != 0)
        {
            eep_print("ll_write() can not get ACK in %s()\n", __FUNCTION__);
            return FAIL_CMDSEND;
        }
    }

    eep_print("%s() send command success\n", __FUNCTION__);
    return SUCCESS;
}

/** \brief Receive ucLen bytes over the bus.
 * 
 * \param pucRecBuf is a pointer to a buffer to contain ucLen bytes received.
 * \param ucLen is the number of bytes to receive
 *
 * \return ::SUCCESS 
 */
uchar ll_ReceiveData(puchar pucRecBuf, uchar ucLen)
{
    if (ucLen > 0)
    {
        while (--ucLen)
        {
            *pucRecBuf++ = ll_Read();
            ll_AckNak(TRUE);
        }
        *pucRecBuf = ll_Read();
        ll_AckNak(FALSE);
    }
    ll_Stop();
    return SUCCESS;
}

/** \brief Send data by calling ll_Write() and then sends Stop with ll_Stop().
 * 
 * \param pucSendBuf is a pointer to a buffer containing ucLen bytes to send
 * \param ucLen is the number of bytes to send
 *
 * \return ::SUCCESS or ::FAIL_WRDATA (if ll_Write() fails)
 */
uchar ll_SendData(puchar pucSendBuf, uchar ucLen)
{
    int i;
    for (i = 0; i < ucLen; i++)
    {
        if (ll_Write(pucSendBuf[i]) != 0)
        {
            eep_print("ll_write() can not get ACK in %s()\n", __FUNCTION__);
            return FAIL_WRDATA;
        }
    }
    // Even when ucLen = 0 (no bytes to send) always send a STOP
    ll_Stop();
    return SUCCESS;
}

/** \brief Sends loop * (Start + 15 clocks + Stop) over the bus.
 */
void ll_WaitClock(uchar loop)
{
    ll_ClockCycles(loop);
}
