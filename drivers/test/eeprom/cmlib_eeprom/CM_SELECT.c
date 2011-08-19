// Select Device
//

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"
#include "CM_GPA.H"

// -------------------------------------------------------------------------------------------------
// Data
// -------------------------------------------------------------------------------------------------

uchar ucCM_DevGpaRegisters[16][Gpa_Regs];
uchar ucCM_DevEncrypt[16];
uchar ucCM_DevAuthenticate[16];
uchar ucCM_DevUserZone[16];
uchar ucCM_DevAntiTearing[16];

// -------------------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------------------

// Select Chip
uchar cm_SelectChip(uchar ucChipNew)
{
    uchar ucChip, i;

    if (ucChipNew == 0xFF)
    {
        // Clear All State
        for (ucChip = 0; ucChip < 16; ++ucChip)
        {
            for (i = 0; i < 16; ++i)
                ucCM_DevGpaRegisters[ucChip][i] = 0;
            ucCM_DevEncrypt[ucChip] = 0;
            ucCM_DevAuthenticate[ucChip] = 0;
            ucCM_DevUserZone[ucChip] = 0;
            ucCM_DevAntiTearing[ucChip] = 0;
        }
    }
    else
    {
        ucChip = CM_PORT_CFG.ucChipSelect;
        if (ucChip != ucChipNew)
        {
            // Set Chip Select
            CM_PORT_CFG.ucChipSelect = ucChipNew;

            // Shift Addresses
            ucChip = (ucChip >> 4) & 0xF0;
            ucChipNew = (ucChipNew >> 4) & 0xF0;

            // Swap GPA Registers
            for (i = 0; i < 16; ++i)
            {
                ucCM_DevGpaRegisters[ucChip][i] = ucGpaRegisters[i];
                ucGpaRegisters[i] = ucCM_DevGpaRegisters[ucChipNew][i];
            }

            //Save State 
            ucCM_DevEncrypt[ucChip] = ucCM_Encrypt;
            ucCM_DevAuthenticate[ucChip] = ucCM_Authenticate;
            ucCM_DevUserZone[ucChip] = ucCM_UserZone;
            ucCM_DevAntiTearing[ucChip] = ucCM_AntiTearing;

            // Restore Saved State
            ucCM_Encrypt = ucCM_DevEncrypt[ucChipNew];
            ucCM_Authenticate = ucCM_DevAuthenticate[ucChipNew];
            ucCM_UserZone = ucCM_DevUserZone[ucChipNew];
            ucCM_AntiTearing = ucCM_DevAntiTearing[ucChipNew];
        }
    }

    return SUCCESS;
}
