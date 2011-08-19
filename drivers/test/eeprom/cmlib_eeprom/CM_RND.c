// Low quality random number generator

#include <stdlib.h>
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"

void cm_RandGen(puchar pucRanddat)
{
    uchar i;

//  srand(2);                      // need to introduce a source of entrophy
    for (i = 0; i < 8; i++)
        pucRanddat[i] = (uchar) rand();
}
