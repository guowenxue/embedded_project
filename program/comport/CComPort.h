#ifndef COM_PORT_H
#define COM_PORT_H

#include <stdarg.h>

#if defined (LINUX) || defined (UCLINUX)

	#include <termios.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <sys/ioctl.h>
	#include <sys/signal.h>
	#include <sys/types.h>

#endif

#include "GMaster.h"

typedef void (*COM_FRAG_DELAY) (void *p_pvPtr);

typedef struct COM_PORT_T
{
	unsigned char	ucDataBit,
					ucStopBit,
					ucParity,
					ucFlowCtrl,
					ucIsConnected;

	int				iFragSize;

	long			lBaudrate;

	char			acDevFile [MAX_TINY_BUFFER_SZ];

	#if defined (WIN32)

		HANDLE		hComPort;

	#elif defined (LINUX) || defined (UCLINUX)

		int			hComPort;

	#endif

	COM_FRAG_DELAY		pfFragDelay;

	void				*pvBasket;
}
COM_PORT;

void comport_init (COM_PORT *p_pstComPort, char *p_pcDevFile, long p_lBaudrate, unsigned char p_ucDataBit, unsigned char p_ucStopBit, unsigned char p_ucParity, unsigned char p_ucFlowCtrl, int p_iFragSize, COM_FRAG_DELAY p_pfFragDelay, void *p_pvBasket);
void comport_term (COM_PORT *p_pstComPort);

unsigned char comport_open (COM_PORT *p_pstComPort);
void comport_close (COM_PORT *p_pstComPort);

unsigned char comport_send (COM_PORT *p_pstComPort, unsigned char *p_pucData, int p_iLen);
unsigned char comport_recv (COM_PORT *p_pstComPort, unsigned char *p_pucData, int p_iSize, int *p_piLen, unsigned long p_ulTimeout);

#endif
