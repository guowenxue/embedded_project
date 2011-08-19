// Verify Password
//

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"
#include "CM_GPA.H"

uchar ucCmdPassword[4] = { 0xba, 0x00, 0x00, 0x03 };
uchar ucPSW[3];

// Verify Password
uchar cm_VerifyPassword(uchar ucChipAddr, puchar pucPassword, uchar ucSet, uchar ucRW)
{
    uchar i, j;
    uchar ucReturn;
    uchar ucAddr;

    ucCmdPassword[0] = (0xa | ((ucChipAddr & 0xf) << 4));
    // Build command and PAC address
    ucAddr = CM_PSW + (ucSet << 3);
    ucCmdPassword[1] = ucSet;
    if (ucRW != CM_PWWRITE)
    {
        ucCmdPassword[1] |= 0x10;
        ucAddr += 4;
    }
    // Deal with encryption if in authenticate mode
    for (j = 0; j < 3; j++)
    {
        // Encrypt the password
        if (ucCM_Authenticate)
        {
            for (i = 0; i < 5; i++)
                cm_GPAGen(pucPassword[j]);
            ucPSW[j] = Gpa_byte;
        }
        // Else just copy it
        ucPSW[j] = pucPassword[j];
    }

    // Send the command
    ucReturn = cm_WriteCommand(ucCmdPassword, ucPSW, 3);

    // Wait for chip to process password
    CM_LOW_LEVEL.WaitClock(3);

    // Read Password attempts counter to determine if the password was accepted
    if (ucReturn == SUCCESS)
    {
        ucReturn = cm_ReadConfigZone(ucChipAddr, ucAddr, ucPSW, 1);
        if (ucPSW[0] != 0xFF)
            ucReturn = FAILED;
    }
    if (ucCM_Authenticate && (ucReturn != SUCCESS))
        cm_ResetCrypto();

    // Done
    return ucReturn;
}
