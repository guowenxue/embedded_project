// Mid Level Utility Function: cm_WriteCommand()
//
// Note: this module must be after all low level functions in the library and
//       before all high level user function to assure that any reference to
//       this function in this library are satistified.

#include "CM_LIB.H"
#include "CM_I2C.H"

uchar cm_WriteCommand(puchar pucInsBuff, puchar pucSendVal, uchar ucLen)
{
    uchar ucReturn;

    if ((ucReturn = CM_LOW_LEVEL.SendCommand(pucInsBuff)) != SUCCESS)
        return ucReturn;
    return CM_LOW_LEVEL.SendData(pucSendVal, ucLen);
}
