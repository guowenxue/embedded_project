
#include "pcscdefines.h"
#include "ifdhandler.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define SCARD_PROTOCOL_T0       0

int main(int argc, char **argv)
{
	unsigned char pAtr[50];
	unsigned long AtrLength = 0;
	int i = 0, j = 0, count = 0;
	unsigned char pbApdu[270];
	unsigned char pbResp[270];
	unsigned long iApduLen = 0;
	unsigned long iRespLen = 270;
	SCARD_IO_HEADER SendPci, RecvPci;

	SendPci.Protocol = SCARD_PROTOCOL_T0;
	SendPci.Length = 0;

	memset(&RecvPci, 0, sizeof(SCARD_IO_HEADER));

    count = 1;

	printf("\n Test IFDHCreateChannel       :\n");
	IFDHCreateChannel(0, 0);

    long present;
    present = IFDHICCPresence(0);
    printf("SAM card present status: %ld\n", present);
    if(IFD_ICC_PRESENT != present)
    {
           return -1; 
    }

	printf("\nTest IFDHPowerICC    IFD_RESET       :\n");
	memset(pAtr, 0, 50);
	if (0 != IFDHPowerICC(0, IFD_RESET, pAtr, &AtrLength))
	{
		printf("IFDHPowerICC error\n");
		goto ret;
	}
	printf("Recv %d bytes Atr:\n", AtrLength);
	for (j = 0; j < AtrLength; j++)
		printf("%02x ", pAtr[j]);
	printf("Over\n\n");

	for (i = 0; i < count; i++)
	{
printf("####################################################################\n");
		printf("\nTest  IFDHSetProtocolParameters\n");
		IFDHSetProtocolParameters(0, 1, 0, 0, 0, 0);

           int k = 0;
           for(k=0; k<3; k++)
           {
		printf("\nTest IFDHTransmitToICC          : \n");
		iApduLen = 0;
		pbApdu[iApduLen++] = 0x00;
		pbApdu[iApduLen++] = 0xA4;
		pbApdu[iApduLen++] = 0x04;
		pbApdu[iApduLen++] = 0x0C;
		pbApdu[iApduLen++] = 0x07;
		pbApdu[iApduLen++] = 0xA0;
		pbApdu[iApduLen++] = 0x00;
		pbApdu[iApduLen++] = 0x00;
		pbApdu[iApduLen++] = 0x47;
		pbApdu[iApduLen++] = 0x53;
		pbApdu[iApduLen++] = 0x01;
		pbApdu[iApduLen++] = 0x01;

		printf(">>> ");
		for (j = 0; j < iApduLen; j++)
			printf("%02X ", pbApdu[j]);
		printf("\n");

		IFDHTransmitToICC(0, SendPci, pbApdu, iApduLen, pbResp, &iRespLen, &RecvPci);
		printf("<<< ");
		for (j = 0; j < iRespLen; j++)
			printf("%02X ", pbResp[j]);
		printf("\n");
           }

// Power down
        printf("\nTest IFDHPowerICC Power Down          :\n");
	memset(pAtr, 0, 50);
	if (0 != IFDHPowerICC(0, IFD_POWER_DOWN, pAtr, &AtrLength))
	{
		printf("IFDHPowerICC error\n");
		goto ret;
	}
	printf("Recv %d bytes Atr:\n", AtrLength);
	for (j = 0; j < AtrLength; j++)
		printf("%02x ", pAtr[j]);
	printf("Over\n\n");

        printf("\nTest IFDHPowerICC Power Up          :\n");
	memset(pAtr, 0, 50);
	if (0 != IFDHPowerICC(0, IFD_POWER_UP, pAtr, &AtrLength))
//	if (0 != IFDHPowerICC(0, IFD_RESET, pAtr, &AtrLength))
	{
		printf("IFDHPowerICC error\n");
		goto ret;
	}
	printf("Recv %d bytes Atr:\n", AtrLength);
	for (j = 0; j < AtrLength; j++)
		printf("%02x ", pAtr[j]);
	printf("Over\n");

	printf("\nTest IFDHPowerICC Card Reset          :\n");
	memset(pAtr, 0, 50);
	if (0 != IFDHPowerICC(0, IFD_RESET, pAtr, &AtrLength))
	{
		printf("IFDHPowerICC error\n");
		goto ret;
	}
	printf("Recv %d bytes Atr:\n", AtrLength);
	for (j = 0; j < AtrLength; j++)
		printf("%02x ", pAtr[j]);
	printf("Over\n\n");



printf("####################################################################\n");
	}

	printf("Close channel.\n");
	IFDHCloseChannel(0);

	return 0;
ret:
	printf("Error return now.\n");
	return 0;
}
