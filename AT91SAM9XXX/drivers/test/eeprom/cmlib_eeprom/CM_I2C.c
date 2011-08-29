// Functions that directly control the hardware

#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"

#if 0
// 1/2 Clock Cycle transition to HIGH
//
void cm_Clockhigh(void)
{
    cm_Delay(1);
    CM_CLK_HI;
    cm_Delay(1);
}

// 1/2 Clock Cycle transition to LOW
//
void cm_Clocklow(void)
{
    cm_Delay(1);
    CM_CLK_LO;
    cm_Delay(1);
}

// Do one full clock cycle
//
// Changed 1/19/05 to eliminate one level of return stack requirements
//
void cm_ClockCycle(void)
{
    cm_Delay(1);
    CM_CLK_LO;
    cm_Delay(2);
    CM_CLK_HI;
    cm_Delay(1);
}

// Do a number of clock cycles
//
void cm_ClockCycles(uchar ucCount)
{
    uchar i;

    for (i = 0; i < ucCount; ++i)
        cm_ClockCycle();
}

// Send a start sequence
//
// Modified 7-21-04 to correctly set SDA to be an output
// 
void cm_Start(void)
{
    CM_DATA_OUT;                // Data line must be an output to send a start sequence
    cm_Clocklow();
    CM_DATA_HI;
    cm_Delay(4);
    cm_Clockhigh();
    cm_Delay(4);
    CM_DATA_LO;
    cm_Delay(8);
    cm_Clocklow();
    cm_Delay(8);
}

// Send a stop sequence
//
// Modified 7-21-04 to correctly set SDA to be an output
// 
void cm_Stop(void)
{
    CM_DATA_OUT;                // Data line must be an output to send a stop sequence
    cm_Clocklow();
    CM_DATA_LO;
    cm_Delay(4);
    cm_Clockhigh();
    cm_Delay(8);
    CM_DATA_HI;
    cm_Delay(4);
}

// Write a byte
//
// Returns 0 if write successed, 1 if write fails failure
//
// Modified 7-21-04 to correctly control SDA
// 
uchar cm_Write(uchar ucData)
{
    uchar i;

    CM_DATA_OUT;                // Set data line to be an output
    for (i = 0; i < 8; i++)
    {                           // Send 8 bits of data
        cm_Clocklow();
        if (ucData & 0x80)
            CM_DATA_HI;
        else
            CM_DATA_LO;
        cm_Clockhigh();
        ucData = ucData << 1;
    }
    cm_Clocklow();

    // wait for the ack
    CM_DATA_IN;                 // Set data line to be an input
    cm_Delay(8);
    cm_Clockhigh();
    while (i > 1)
    {                           // loop waiting for ack (loop above left i == 8)
        cm_Delay(2);
        if (CM_DATA_RD)
            i--;                // if SDA is high level decrement retry counter
        else
            i = 0;
    }
    cm_Clocklow();
    CM_DATA_OUT;                // Set data line to be an output
    return i;
}

// Send a ACK or NAK or to the device
void cm_AckNak(uchar ucAck)
{
    CM_DATA_OUT;                // Data line must be an output to send an ACK
    cm_Clocklow();
    if (ucAck)
        CM_DATA_LO;             // Low on data line indicates an ACK
    else
        CM_DATA_HI;             // High on data line indicates an NACK
    cm_Delay(2);
    cm_Clockhigh();
    cm_Delay(8);
    cm_Clocklow();
}

#ifdef PIGS_FLY

// ------------------------------------------------------------------------------------- 
// Original Version
// ------------------------------------------------------------------------------------- 

// Send a ACK to the device
void cm_Ack(void)
{
    CM_DATA_OUT;                // Data line must be an output to send an ACK
    cm_Clocklow();
    CM_DATA_LO;                 // Low on data line indicates an ACK
    cm_Delay(2);
    cm_Clockhigh();
    cm_Delay(8);
    cm_Clocklow();
    //SET_SDA;
}

// Send a NACK to the device
void cm_N_Ack(void)
{
    CM_DATA_OUT;                // Data line must be an output to send an NACK
    cm_Clocklow();
    CM_DATA_HI;                 // High on data line indicates an NACK
    cm_Delay(2);
    cm_Clockhigh();
    cm_Delay(8);
    cm_Clocklow();
    //SET_SDA;
}

// ------------------------------------------------------------------------------------- 
// Version that uses one less level of call stack
// ------------------------------------------------------------------------------------- 

// Send a ACK to the device
void cm_Ack(void)
{
    CM_DATA_OUT;                // Data line must be an output to send an ACK
    cm_Delay(1);
    CM_CLK_LO;
    cm_Delay(1);
    CM_DATA_LO;                 // Low on data line indicates an ACK
    cm_Delay(3);
    CM_CLK_HI;
    cm_Delay(9);
    cm_Clocklow();
}

// Send a NACK to the device
void cm_N_Ack(void)
{
    CM_DATA_OUT;                // Data line must be an output to send an NACK
    cm_Delay(1);
    CM_CLK_LO;
    cm_Delay(1);
    CM_DATA_HI;                 // High on data line indicates an NACK
    cm_Delay(2);
    CM_CLK_HI;
    cm_Delay(9);
    cm_Clocklow();
}
#endif

//     Read a byte from device, MSB
//
// Modified 7-21-04 to correctly control SDA
// 
uchar cm_Read(void)
{
    uchar i;
    uchar rByte = 0;

    CM_DATA_IN;                 // Set data line to be an input
    CM_DATA_HI;
    for (i = 0x80; i; i = i >> 1)
    {
        cm_ClockCycle();
        if (CM_DATA_RD)
            rByte |= i;
        cm_Clocklow();
    }
    CM_DATA_OUT;                // Set data line to be an output
    return rByte;
}
#endif

void cm_WaitClock(uchar loop)
{
#if 0
    uchar i, j;

    CM_DATA_LO;
    for (j = 0; j < loop; j++)
    {
        cm_Start();
        for (i = 0; i < 15; i++)
            cm_ClockCycle();
        cm_Stop();
    }
#endif
    ll_WaitClock(loop);
}

// Send a command
//
uchar cm_SendCommand(puchar pucInsBuff)
{
#if 0
    uchar i, ucCmd;

    i = CM_START_TRIES;
    ucCmd = (pucInsBuff[0] & 0x0F) | CM_PORT_CFG.ucChipSelect;
    while (i)
    {
        cm_Start();
        if (cm_Write(ucCmd) == 0)
            break;
        if (--i == 0)
            return FAIL_CMDSTART;
    }

    for (i = 1; i < 4; i++)
    {
        if (cm_Write(pucInsBuff[i]) != 0)
            return FAIL_CMDSEND;
    }
    return SUCCESS;
#endif
    return ll_SendCommand(pucInsBuff);
}

uchar cm_ReceiveData(puchar pucRecBuf, uchar ucLen)
{
#if 0
    int i;

    for (i = 0; i < (ucLen - 1); i++)
    {
        pucRecBuf[i] = cm_Read();
        cm_AckNak(TRUE);
    }
    pucRecBuf[i] = cm_Read();
    cm_AckNak(FALSE);
    cm_Stop();
    return SUCCESS;
#endif
    return ll_ReceiveData(pucRecBuf, ucLen);
}

uchar cm_SendData(puchar pucSendBuf, uchar ucLen)
{
#if 0
    int i;
    for (i = 0; i < ucLen; i++)
    {
        if (cm_Write(pucSendBuf[i]) == 1)
            return FAIL_WRDATA;
    }
    cm_Stop();

    return SUCCESS;
#endif
    return ll_SendData(pucSendBuf, ucLen);
}
