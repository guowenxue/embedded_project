// Set User Zone
//

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_GPA.H"
#include <stdio.h>

// Set User Zone
uchar cm_SetUserZone(uchar ucChipAddr, uchar ucZoneNumber, uchar ucAntiTearing)
{
    uchar ucReturn;

    ucCM_InsBuff[0] = (0x4 | ((ucChipAddr & 0xf) << 4));
    if (ucAntiTearing)
        ucCM_InsBuff[1] = 0x0b;
    else
        ucCM_InsBuff[1] = 0x03;
    ucCM_InsBuff[2] = ucZoneNumber;
    ucCM_InsBuff[3] = 0x00;

    // Only zone number is included in the polynomial
    cm_GPAGen(ucZoneNumber);

    if ((ucReturn = CM_LOW_LEVEL.SendCommand(ucCM_InsBuff)) != SUCCESS)
    {
        printf("Send Command failure: 0x%0x\n", ucReturn);
        return ucReturn;
    }
    // save zone number and anti-tearing state
    ucCM_UserZone = ucZoneNumber;
    ucCM_AntiTearing = ucAntiTearing;

    // done 
    return CM_LOW_LEVEL.ReceiveRet(NULL, 0);
}
