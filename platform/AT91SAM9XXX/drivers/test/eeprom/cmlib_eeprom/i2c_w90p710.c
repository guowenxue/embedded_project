
/*W90710 is ARM7, we will simulator the I2C protocol driver here(in user space)*/

#if (defined CPU_W90P710)
#include "ll_port.h"

#define GPIO_CFG5       0xfff83050
#define GPIO_DIR5       0xfff83054
#define GPIO_DATAOUT5   0xfff83058
#define GPIO_DATAIN5    0xfff8305c

/* Atmel EEPROM AT88SC0104C */
#define SCLK            (1<<6)  /* GPIO11, PORT5.6 */
#define SDAT            (1<<7)  /* GPIO12, PORT5.7 */

#define GPIO_MODE       0x00

#define LOWLEVEL        0       /*Set GPIO port as low level */
#define HIGHLEVEL       1       /*Set GPIO port as high level */

#define INPUT           0       /* Set GPIO port direction as input mode */
#define OUTPUT          1       /* Set GPIO port direction as output mode */

#define ENPULLUP        0x1     /*Enable GPIO port internal pull up resister */
#define DISPULLUP       0x0     /*Disable GPIO port internal pull up resister */

#define inpl(adr)               (*(unsigned long volatile *)(adr))
#define outpl(adr, val)         (*(unsigned long volatile *)(adr)= val)

#define setbit(var,n)           (var)=( (var) | 1<<(n) )
#define clearbit(var,n)         (var)=( (var) & (~(1<<(n))) )

/**********************************************************************************
 *           Basic GPIO operate function
 *********************************************************************************/

/* Get which bit is set be 1*/
static inline int get_bit(const unsigned long val)
{
    int i = 1;
    if (!val)
        return 0;

    while (val >> i)            /*Right shift untill all bit is zero */
        i++;

    return --i;                 /*Last shift bit is set as 1 */
}

/*************************************************************************************
 *            Set the Direction Register
 * port_reg:  which group, such as GPIO_CFG5, GPIO_CFG6 defined  in l200_gpio.h 
 *     port:  which port, such as: UC864_VBUS defined in l200_gpio.h
 *      dir:  OUTPUT or INPUT
 *   pullup:  DISPULLUP or ENPULLUP 
 ************************************************************************************/
void inline set_gpio_dir(unsigned long port_reg, unsigned long port, int dir, int pullup)
{
    unsigned long port_val = inpl(port_reg);

    /* set direction */
    if (dir & 0x1)
        setbit(port_val, get_bit(port));
    else
        clearbit(port_val, get_bit(port));

    // pullup is start begain bit: 1<<16
    if (pullup & 0x1)
        setbit(port_val, 16 + get_bit(port));
    else
        clearbit(port_val, 16 + get_bit(port));

    outpl(port_reg, port_val);
}

/*************************************************************************************
 *           Set the Configure Register mode
 * port_reg: which group, such as GPIO_CFG5, GPIO_CFG6 defined  in l200_gpio.h 
 *     port:  which port, such as: UC864_VBUS defined in l200_gpio.h
 *      mode:  0x00, 0x01, 0x10, 0x11:   0x00->GPIO mode 
 ************************************************************************************/
void set_gpio_cfg(unsigned long port_reg, unsigned long port, unsigned int mode)
{
    unsigned long port_val = inpl(port_reg);

    if (mode & 0x01)
        setbit(port_val, 2 * get_bit(port));
    else
        clearbit(port_val, 2 * get_bit(port));

    if (mode & 0x10)
        setbit(port_val, 1 + 2 * get_bit(port));
    else
        clearbit(port_val, 1 + 2 * get_bit(port));

    outpl(port_reg, port_val);
}

/*************************************************************************************
 * Set the DATAOUT port as High Level or Low Level
 * port:  which port, such as: UC864_VBUS defined in l200_gpio.h
 *  val:  HIGHLEVEL or LOWLEVEL
 ************************************************************************************/
void set_port_value(unsigned long port_reg, unsigned long port, unsigned int val)
{
    unsigned long port_val = inpl(port_reg);

    if (val & 0x1)
        setbit(port_val, get_bit(port));
    else
        clearbit(port_val, get_bit(port));

    outpl(port_reg, port_val);
}

/*************************************************************************************
 * Get the DATAOUT/DATAIN port Power Level 
 * port:  which port, such as: UC864_VBUS defined in l200_gpio.h
 ************************************************************************************/
int get_port_value(unsigned long port_reg, unsigned long port)
{
    unsigned long port_val = inpl(port_reg);

    if (port_val & port)
        return HIGHLEVEL;
    else
        return LOWLEVEL;
}

/**********************************************************************************
 *           I2C protocol simulator start
 *********************************************************************************/

void i2c_init(void)
{
    set_gpio_cfg(GPIO_CFG5, SCLK, GPIO_MODE);
    set_gpio_cfg(GPIO_CFG5, SDAT, GPIO_MODE);
}

void i2c_Delay(uchar ucDelay)
{
    int i, j;
    for (i = 0; i < ucDelay; i++)
        for (j = 0; j < 20; j++) ;  // 1 us here ? no test
}

/** \brief Sets the clock pin to a high level by floating the port output
 *
 *  See file ll_port.h for port structure definitions.
 *  Uses the internal port pull-up to provide the TWI clock pull-up.
 */
void i2c_Clockhigh(void)
{
    i2c_Delay(1);

    set_gpio_dir(GPIO_DIR5, SCLK, INPUT, DISPULLUP);
    set_port_value(GPIO_DATAOUT5, SCLK, HIGHLEVEL);

    i2c_Delay(1);
}

/** \brief Sets the clock pin to a low level. 
 *
 *  See file ll_port.h for port structure definitions.
 */
void i2c_Clocklow(void)
{

    i2c_Delay(1);
    set_port_value(GPIO_DATAOUT5, SCLK, LOWLEVEL);

    i2c_Delay(2);
    set_gpio_dir(GPIO_DIR5, SCLK, OUTPUT, DISPULLUP);
}

/** \brief Cycles the clock pin low then high. */
void i2c_ClockCycle(void)
{
    i2c_Clocklow();
    i2c_Clockhigh();
}

/// \brief Calls i2c_ClockCycle() ucCount times
void i2c_ClockCycles(uchar ucCount)
{
    for (; ucCount > 0; ucCount--)
        i2c_ClockCycle();
}

/** \brief Sets the data pin to a high level by floating the port output
 *
 *  See file ll_port.h for port structure definitions.
 *  Uses the internal port pull-up to provide the TWI data pull-up.
 */
void i2c_Datahigh(void)
{
    i2c_Delay(1);

    set_gpio_dir(GPIO_DIR5, SDAT, INPUT, DISPULLUP);
    set_port_value(GPIO_DATAOUT5, SDAT, HIGHLEVEL);

    i2c_Delay(2);
}

/** \brief Sets the data pin to a low level. 
 *
 *  See file ll_port.h for port structure definitions.
 */
void i2c_Datalow(void)
{
    i2c_Delay(1);
    set_port_value(GPIO_DATAOUT5, SDAT, LOWLEVEL);

    i2c_Delay(2);
    set_gpio_dir(GPIO_DIR5, SDAT, OUTPUT, DISPULLUP);
}

/** \brief Reads and returns the data pin value.  Leaves the pin high-impedance.
*/
uchar i2c_Data(void)
{
    i2c_Delay(1);

    set_gpio_dir(GPIO_DIR5, SDAT, INPUT, DISPULLUP);
    set_port_value(GPIO_DATAOUT5, SDAT, HIGHLEVEL);

    i2c_Delay(4);

    return get_port_value(GPIO_DATAIN5, SDAT);
}

/** \brief Sends a start sequence */
void i2c_Start(void)
{
    i2c_Clocklow();
    i2c_Datahigh();
    i2c_Delay(4);
    i2c_Clockhigh();
    i2c_Delay(4);
    i2c_Datalow();
    i2c_Delay(4);
    i2c_Clocklow();
    i2c_Delay(4);
}

/** \brief  Sends a stop sequence */
void i2c_Stop(void)
{
    i2c_Clocklow();
    i2c_Datalow();
    i2c_Clockhigh();
    i2c_Delay(8);
    i2c_Datahigh();
    i2c_Delay(4);
}

/** \brief Sends an ACK or NAK to the device (after a read). 
 * 
 *  \param ucAck - if ::TRUE means an ACK will be sent, otherwise
 *  NACK sent.
 */
void i2c_AckNak(uchar ucAck)
{
    i2c_Clocklow();
    if (ucAck)
    {
        i2c_Datalow();          //Low data line indicates an ACK
    }
    else
    {
        i2c_Datahigh();         // High data line indicates an NACK
    }
    i2c_Clockhigh();
    i2c_Clocklow();
}

/** \brief Power On chip sequencing.  Clocks the chip ::LL_PWRON_CLKS times. 
*/
void i2c_PowerOff(void)
{
    i2c_Delay(1);
    i2c_Clocklow();
    i2c_Delay(6);
}

void i2c_PowerOn(void)
{
    // Sequence for powering on secure memory according to ATMEL spec
    i2c_Datahigh();             // Data high during reset
    i2c_Clocklow();             // Clock should start LOW
    i2c_ClockCycles(LL_PWRON_CLKS);
    // Give chip some clocks cycles to get started
    // Chip should now be in sync mode and ready to operate
}

/** \brief  Write a byte on the TWI-like bus.
*
* \return 0 if write success, else 'n' (the number of attempts) 
*  to get ACK that failed (maximum is ::LL_ACK_TRIES).
*/
uchar i2c_Write(uchar ucData)
{
    uchar i;

    for (i = 0; i < 8; i++)
    {                           // Send 8 bits of data
        i2c_Clocklow();
        if (ucData & 0x80)
            i2c_Datahigh();
        else
            i2c_Datalow();
        i2c_Clockhigh();
        ucData = ucData << 1;
    }

    i2c_Clocklow();

    // wait for the ack
    i2c_Datahigh();             // Set data line to be an input
    i2c_Delay(8);
    i2c_Clockhigh();
    for (i = 0; i < LL_ACK_TRIES; i++)
    {
        if (i2c_Data() == 0)
        {
            i = 0;
            break;
        }
    }
    i2c_Clocklow();

    return i;
}

/** \brief Read a byte from device, MSB first, no ACK written yet*/
uchar i2c_Read(void)
{
    uchar i;
    uchar rByte = 0;

    i2c_Datahigh();

    for (i = 0x80; i; i = i >> 1)
    {
        i2c_ClockCycle();

        if (i2c_Data())
            rByte |= i;

        i2c_Clocklow();
    }

    return rByte;
}

/** \brief Send a command (usually 4 bytes) over the bus.
 * 
 * \param pucInsBuf is pointer to a buffer containing numBytes to send
 * \param numBytes is the number of bytes to send
 *
 * \return ::SUCCESS or ::FAIL_CMDSEND (if i2c_Write() fails)
 */
uchar i2c_SendCommand(puchar pucInsBuf)
{
    uchar numBytes = 4;
    uchar i = LL_START_TRIES;

    while (i)
    {
        i2c_Start();

        if (i2c_Write(pucInsBuf[0]) == 0)
        {
            break;
        }

        if (--i == 0)
        {
            return FAIL_CMDSTART;
        }
    }

    for (i = 1; i < numBytes; i++)
    {
        if (i2c_Write(pucInsBuf[i]) != 0)
        {
            return FAIL_CMDSEND;
        }
    }
    return SUCCESS;
}

/** \brief Receive ucLen bytes over the bus.
 * 
 * \param pucRecBuf is a pointer to a buffer to contain ucLen bytes received.
 * \param ucLen is the number of bytes to receive
 *
 * \return ::SUCCESS 
 */
uchar i2c_ReceiveData(puchar pucRecBuf, uchar ucLen)
{
    if (ucLen > 0)
    {
        while (--ucLen)
        {
            *pucRecBuf = i2c_Read();
            pucRecBuf++;

            i2c_AckNak(TRUE);
        }

        *pucRecBuf = i2c_Read();
        i2c_AckNak(FALSE);
    }

    i2c_Stop();

    return SUCCESS;
}

/** \brief Send data by calling i2c_Write() and then sends Stop with i2c_Stop().
 * 
 * \param pucSendBuf is a pointer to a buffer containing ucLen bytes to send
 * \param ucLen is the number of bytes to send
 *
 * \return ::SUCCESS or ::FAIL_WRDATA (if i2c_Write() fails)
 */
uchar i2c_SendData(puchar pucSendBuf, uchar ucLen)
{
    int i;

    for (i = 0; i < ucLen; i++)
    {
        if (i2c_Write(pucSendBuf[i]) != 0)
        {
            return FAIL_WRDATA;
        }
    }

    // Even when ucLen = 0 (no bytes to send) always send a STOP
    i2c_Stop();

    return SUCCESS;
}

/** \brief Sends loop * (Start + 15 clocks + Stop) over the bus.
 */
void i2c_WaitClock(uchar loop)
{
    uchar i, j;

    set_port_value(GPIO_DATAOUT5, SDAT, LOWLEVEL);

    for (j = 0; j < loop; j++)
    {
        i2c_Start();

        for (i = 0; i < 15; i++)
            i2c_ClockCycle();

        i2c_Stop();
    }
}

#endif                          /*End of CPU_W90P710 */
