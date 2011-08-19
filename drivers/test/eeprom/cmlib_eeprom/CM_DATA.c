// Data and Configuration for the CryptoMemory Library
#include "CM_LIB.H"
#include "CM_I2C.H"

// Data Structures that configure the low level CryptoMemory functions

// CryptoMemory Low Level Linkage
// 
cm_low_level CM_LOW_LEVEL = {
    cm_TRUE,                    // Carddetect
    cm_PowerOff,                // PowerOff
    cm_PowerOn,                 // PowerOn
    cm_SendCommand,             // SendCommand
    cm_ReceiveData,             // ReceiveRet
    cm_SendData,                // SendData
    cm_RandGen,                 // RandomGen
    cm_WaitClock,               // WaitClock
    cm_SendCmdByte              // SendCmdByte
};

// CryptoMemory Low Level Configuration
//
// Note: the port address is included in a manner that does not require a chip
//       specific header file. Note, the address of the port is the LAST address
//       of the group of three addresses of the port (the port output register).
//
cm_port_cfg CM_PORT_CFG = {
    0xb0,                       // ucChipSelect        (0xb0 is default address for CryptoMemory)
    0x32,                       // ucClockPort         (0x32 is PORTD)
    0,                          // ucClockPin          (SCL on bit 0)
    0x32,                       // ucDataPort          (0x32 is PORTD)
    2,                          // ucDataPin           (SDA on bit 2)    
    0x32,                       // ucCardSensePort     (0x32 is PORTD)
    1,                          // ucCardSensePin      (card sense switch, if any, on bit 2) 
    TRUE,                       // ucCardSensePolarity (TRUE -> "1" on bit in register means card is inserted)
    0x32,                       // ucPowerPort         (0x32 is PORTD)
    3,                          // ucPowerPin          (power control, if any, on bit 3)
    TRUE,                       // ucPowerPolarity     (TRUE -> "1" on bit in register supplies power)
    100,                        // ucDelayCount
    10                          // ucStartTries
};
