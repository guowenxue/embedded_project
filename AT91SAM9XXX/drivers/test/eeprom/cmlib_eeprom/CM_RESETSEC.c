// Deactivate Security
//

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"
#include "CM_GPA.H"

// Deactivate Security
//uchar cm_DeactiveSecurity(void)
uchar cm_DeactiveSecurity(uchar ucChipAddr)
{
    uchar ucReturn;
    uchar cmdByte = (0x8 | ((ucChipAddr & 0xf) << 4));

    //if ((ucReturn = CM_LOW_LEVEL.SendCmdByte(0xb8)) != SUCCESS) return ucReturn;
    if ((ucReturn = CM_LOW_LEVEL.SendCmdByte(cmdByte)) != SUCCESS)
        return ucReturn;

    cm_ResetCrypto();

    return SUCCESS;
}
