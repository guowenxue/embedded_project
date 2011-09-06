#include "CComPort.h"

void comport_init (COM_PORT *p_pstComPort, char *p_pcDevFile, long p_lBaudrate, unsigned char p_ucDataBit, unsigned char p_ucStopBit, unsigned char p_ucParity, unsigned char p_ucFlowCtrl, int p_iFragSize, COM_FRAG_DELAY p_pfFragDelay, void *p_pvBasket)
{
	strncpy (p_pstComPort->acDevFile, p_pcDevFile, sizeof (p_pstComPort->acDevFile) - 1);

	p_pstComPort->lBaudrate = p_lBaudrate;
	p_pstComPort->ucDataBit = p_ucDataBit;
	p_pstComPort->ucStopBit = p_ucStopBit;
	p_pstComPort->ucParity = p_ucParity;
	p_pstComPort->ucFlowCtrl = p_ucFlowCtrl;

	p_pstComPort->iFragSize = (0 >= p_iFragSize) ? 64 : p_iFragSize;

	p_pstComPort->hComPort = 0;
	p_pstComPort->pfFragDelay = p_pfFragDelay;
	p_pstComPort->pvBasket = p_pvBasket;

	p_pstComPort->ucIsConnected = 0x00;
}

void comport_term (COM_PORT *p_pstComPort)
{
	comport_close (p_pstComPort);
	memset (p_pstComPort, 0x00, sizeof (COM_PORT));
}

unsigned char comport_open (COM_PORT *p_pstComPort)
{
	unsigned char			ucRet;

	#if defined (WIN32)

		DCB					stOldCfg,
							stNewCfg;

		COMMTIMEOUTS		stTimeOut;

	#elif defined (LINUX) || defined (UCLINUX)

		struct termios		stOldCfg,
							stNewCfg;

		int					iOldFlags;

		long				lTmp;

	#endif

	comport_close (p_pstComPort);

	#if defined (WIN32)

		// Need to open Com port first to retrieve existing settings
		p_pstComPort->hComPort = CreateFile (p_pstComPort->acDevFile, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (0 == p_pstComPort->hComPort || INVALID_HANDLE_VALUE == p_pstComPort->hComPort)
		{
			p_pstComPort->hComPort = 0;

			ucRet = 0x01;
			goto CleanUp;
		}

		if (0 < p_pstComPort->lBaudrate)
		{
			// Get the existing Com port settings
			if (0 == GetCommState (p_pstComPort->hComPort, &stOldCfg))
			{
				ucRet = 0x06;	// Failed to get Com settings
				goto CleanUp;
			}

			memcpy (&stNewCfg, &stOldCfg, sizeof (stNewCfg));

			// ========================
			// ** Configure Com port **
			// ========================

			stNewCfg.BaudRate = p_pstComPort->lBaudrate;	// Set the Baud rate
			stNewCfg.ByteSize = p_pstComPort->ucDataBit;	// Set the data bit
			stNewCfg.Parity = p_pstComPort->ucParity;		// Set the parity
			stNewCfg.StopBits = p_pstComPort->ucStopBit;	// Set the stop bit
			
			// Set flow control
			switch (p_pstComPort->ucFlowCtrl)
			{
			case 0x00:	// None
				break;

			case 0x01:	// Software control
			case 0x03:	// Both
				stNewCfg.fInX = 1;
				stNewCfg.fOutX = 1;
				break;

			case 2:	// Hardware control
				stNewCfg.fOutxCtsFlow = 1;
				stNewCfg.fRtsControl = RTS_CONTROL_HANDSHAKE;
				break;
			}
			
			stNewCfg.fBinary = TRUE;
			
			// Set the new Com port settings
			if (0 == SetCommState (p_pstComPort->hComPort, &stNewCfg))
			{
				ucRet = 0x07;	// Failed to set Com settings
				goto CleanUp;
			}

			// Set the Com port timeout settings
			stTimeOut.ReadIntervalTimeout = MAXDWORD;
			stTimeOut.ReadTotalTimeoutMultiplier = 0;
			stTimeOut.ReadTotalTimeoutConstant = 0;
			stTimeOut.WriteTotalTimeoutMultiplier = 0;
			stTimeOut.WriteTotalTimeoutConstant = 0;

			if (0 == SetCommTimeouts (p_pstComPort->hComPort, &stTimeOut))
			{
				ucRet = 0x08;	// Failed to set Com timeout settings
				goto CleanUp;
			}
		}

	#elif defined (LINUX) || defined (UCLINUX)

		if (0 >= p_pstComPort->lBaudrate)
		{
			// Open the device to be non-blocking (read will return immediately)
			p_pstComPort->hComPort = open (p_pstComPort->acDevFile, O_RDWR);

			if (0 >= p_pstComPort->hComPort)
			{
				ucRet = 0x05;	// Failed to open device com port
				goto CleanUp;
			}
		}
		else
		{
			// Open the device to be non-blocking (read will return immediately)
			p_pstComPort->hComPort = open (p_pstComPort->acDevFile, O_RDWR | O_NOCTTY | O_NONBLOCK);

			if (0 >= p_pstComPort->hComPort)
			{
				ucRet = 0x05;	// Failed to open device com port
				goto CleanUp;
			}

			if ((-1 != (iOldFlags = fcntl (p_pstComPort->hComPort, F_GETFL, 0))) && (-1 != fcntl (p_pstComPort->hComPort, F_SETFL, iOldFlags & ~O_NONBLOCK)))
			{
				// Flush input and output
				if (-1 == tcflush (p_pstComPort->hComPort, TCIOFLUSH))
				{
					ucRet = 0x06;	// Failed to set device com port settings
					goto CleanUp;
				}
			}

			// Get the existing Com port settings
			if (0 != tcgetattr (p_pstComPort->hComPort, &stOldCfg))
			{
				ucRet = 0x07;	// Failed to get Com settings
				goto CleanUp;
			}

			memset (&stNewCfg, 0, sizeof (stNewCfg));

			// ========================
			// ** Configure Com port **
			// ========================

			//stNewCfg.c_cflag != (CLOCAL | CREAD);
			stNewCfg.c_cflag &= ~CSIZE;
			stNewCfg.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	// Set raw input
			stNewCfg.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
			stNewCfg.c_oflag &= ~(OPOST);

			// Set the data bit
			switch(p_pstComPort->ucDataBit)
			{
			case 0x07:
				stNewCfg.c_cflag |= CS7; break;
			case 0x06:
				stNewCfg.c_cflag |= CS6; break;
			case 0x05:
				stNewCfg.c_cflag |= CS5; break;
			default:	// 0x08
				stNewCfg.c_cflag |= CS8;
			}

			// Set the parity
			switch (p_pstComPort->ucParity)
			{
			case 0x01:	// Odd
				stNewCfg.c_cflag |= (PARENB | PARODD);
				stNewCfg.c_cflag |= (INPCK | ISTRIP);
				break;
			case 0x02:	// Even
				stNewCfg.c_cflag |= PARENB;
				stNewCfg.c_cflag &= ~PARODD;
				stNewCfg.c_cflag |= (INPCK | ISTRIP);
				break;
			case 0x03:	// Space
				stNewCfg.c_cflag &= ~PARENB;
				stNewCfg.c_cflag &= ~CSTOPB;
			default:	// None
				stNewCfg.c_cflag &= ~PARENB;
			}

			// Set the stop bit
			if (0x01 != p_pstComPort->ucStopBit)
			{
				stNewCfg.c_cflag |= CSTOPB;
			}
			else
			{
				stNewCfg.c_cflag &= ~CSTOPB;
			}

			// Set flow control
			switch(p_pstComPort->ucFlowCtrl)
			{
			case 1:	// Software control
			case 3:	// Both
				stNewCfg.c_cflag &= ~(CRTSCTS);
				stNewCfg.c_iflag |= (IXON | IXOFF);
				break;
			case 2:	// Hardware control
				stNewCfg.c_cflag |= CRTSCTS;    // Also called CRTSCTS
				stNewCfg.c_iflag &= ~(IXON | IXOFF);
				break;
			default:	// None
				stNewCfg.c_cflag &= ~(CRTSCTS);
				stNewCfg.c_iflag &= ~(IXON | IXOFF);
				break;
			}

			// Set the (input/output) baud rate
			switch (p_pstComPort->lBaudrate)
			{
			case 115200:	lTmp = B115200; break;
			case 57600:		lTmp = B57600; break;
			case 38400:		lTmp = B38400; break;
			case 19200:		lTmp = B19200; break;
			case 9600:		lTmp = B9600; break;
			case 4800:		lTmp = B4800; break;
			case 2400:		lTmp = B2400; break;
			case 1800:		lTmp = B1800; break;
			case 1200:		lTmp = B1200; break;
			case 600:		lTmp = B600; break;
			case 300:		lTmp = B300; break;
			case 200:		lTmp = B200; break;
			case 150:		lTmp = B150; break;
			case 134:		lTmp = B134; break;
			case 110:		lTmp = B110; break;
			case 75:		lTmp = B75; break;
			case 50:		lTmp = B50; break;
			default:		lTmp = B115200;
			}

			cfsetispeed (&stNewCfg, lTmp);
			cfsetospeed (&stNewCfg, lTmp);

			// Set the Com port timeout settings
			stNewCfg.c_cc [VMIN]  = 0;
			stNewCfg.c_cc [VTIME] = 0;

			tcflush (p_pstComPort->hComPort, TCIFLUSH);

			if (0 != tcsetattr (p_pstComPort->hComPort, TCSANOW, &stNewCfg))
			{
				ucRet = 0x08;	// Failed to set device com port settings
				goto CleanUp;
			}
		}

	#endif

	p_pstComPort->ucIsConnected = 0x01;
	ucRet = 0x00;

CleanUp:

	return ucRet;
}

void comport_close (COM_PORT *p_pstComPort)
{
	if (0 != p_pstComPort->hComPort)
	{
		#if defined (WIN32)

			if (0 != p_pstComPort->hComPort && INVALID_HANDLE_VALUE != p_pstComPort->hComPort)
			{
				CloseHandle (p_pstComPort->hComPort);
			}

		#elif defined (LINUX) || defined (UCLINUX)

			if (0 < p_pstComPort->hComPort)
			{
				close (p_pstComPort->hComPort);
			}

		#endif

		p_pstComPort->hComPort = 0;
	}
}

unsigned char comport_send (COM_PORT *p_pstComPort, unsigned char *p_pucData, int p_iLen)
{
	unsigned char		ucRet,
						*pucPtr,
						*pucEnd;

	#if defined (WIN32)

		unsigned long	ulWritten	= 0;

	#elif defined (LINUX) || defined (UCLINUX)

		int				iSend		= 0;

	#endif

	#if defined (DEBUG)
	
		int				iIndex;

	#endif

	if (0 == p_pucData || 0 > p_iLen)
	{
		ucRet = 0x01;
		goto CleanUp;
	}

	if (0x01 != p_pstComPort->ucIsConnected)
	{
		ucRet = 0x02;	// Com port not connected
		goto CleanUp;
	}

	#if defined (WIN32)

		if (p_pstComPort->iFragSize < p_iLen)
		{
			pucPtr = p_pucData;
			pucEnd = p_pucData + p_iLen;

			do
			{
				if (p_pstComPort->iFragSize < pucEnd - pucPtr)
				{
					if (0 == WriteFile (p_pstComPort->hComPort, pucPtr, p_pstComPort->iFragSize, &ulWritten, 0))
					{
						ucRet = 0x03;
						goto CleanUp;
					}
					else if (ulWritten != (unsigned long) p_pstComPort->iFragSize)
					{
						ucRet = 0x03;
						goto CleanUp;
					}

					pucPtr += p_pstComPort->iFragSize;
				}
				else
				{
					if (0 == WriteFile (p_pstComPort->hComPort, pucPtr, pucEnd - pucPtr, &ulWritten, 0))
					{
						ucRet = 0x03;
						goto CleanUp;
					}
					else if (ulWritten != (unsigned long) (pucEnd - pucPtr))
					{
						ucRet = 0x03;
						goto CleanUp;
					}

					pucPtr += (pucEnd - pucPtr);
				}
			}
			while (pucPtr < pucEnd);
		}
		else
		{
			if (0 == WriteFile (p_pstComPort->hComPort, p_pucData, p_iLen, &ulWritten, 0))
			{
				ucRet = 0x03;
				goto CleanUp;
			}
			else if (ulWritten != (unsigned long) p_iLen)
			{
				ucRet = 0x03;
				goto CleanUp;
			}
		}

	#elif defined (LINUX) || defined (UCLINUX)

		if (p_pstComPort->iFragSize < p_iLen)
		{
			pucPtr = p_pucData;
			pucEnd = p_pucData + p_iLen;

			do
			{
				if (p_pstComPort->iFragSize < pucEnd - pucPtr)
				{
					iSend = write (p_pstComPort->hComPort, pucPtr, p_pstComPort->iFragSize);

					if (0 >= iSend || p_pstComPort->iFragSize != iSend)
					{
						ucRet = 0x03;
						goto CleanUp;
					}

					pucPtr += p_pstComPort->iFragSize;
				}
				else
				{
					iSend = write (p_pstComPort->hComPort, pucPtr, pucEnd - pucPtr);

					if (0 >= iSend || (pucEnd - pucPtr) != iSend)
					{
						ucRet = 0x03;
						goto CleanUp;
					}

					pucPtr += pucEnd - pucPtr;
				}

				// Wait until send finished
				if (0 != p_pstComPort->pfFragDelay)
				{
					(p_pstComPort->pfFragDelay) (p_pstComPort->pvBasket);
				}
			}
			while (pucPtr < pucEnd);
		}
		else
		{
			iSend = write (p_pstComPort->hComPort, p_pucData, p_iLen);

			if (0 >= iSend || p_iLen != iSend)
			{
				ucRet = 0x03;
				goto CleanUp;
			}

			// Wait until send finished
			if (0 != p_pstComPort->pfFragDelay)
			{
				(p_pstComPort->pfFragDelay) (p_pstComPort->pvBasket);
			}
		}

	#endif

	ucRet = 0x00;

CleanUp:

	return ucRet;
}

unsigned char comport_recv (COM_PORT *p_pstComPort, unsigned char *p_pucData, int p_iSize, int *p_piLen, unsigned long p_ulTimeout)
{
	unsigned char		ucRet;

	#ifdef WIN32
		// CAUTION: Got one timeout value specified in connect function
		unsigned long	ulCurTime,
						ulStartTime;

	#elif defined (LINUX) || defined (UCLINUX)

		int				iRet;

		fd_set			stReadFds,
						stExcpFds;

		struct timeval	stTime;

	#endif

	#if defined (DEBUG)

		int				iIndex;

	#endif

	if (0 == p_pucData || 0 >= p_iSize || 0 == p_piLen)
	{
		ucRet = 0x02;
		goto CleanUp;
	}
	else
	{
		*p_piLen = 0;
	}

	if (0x01 != p_pstComPort->ucIsConnected)
	{
		ucRet = 0x03;
		goto CleanUp;
	}

	#ifdef WIN32
		// CAUTION: Got one timeout value specified in connect function
		GET_TICKCOUNT (ulCurTime);
		ulStartTime = ulCurTime;

		if (0 < p_ulTimeout)
		{
			while ((ulCurTime - ulStartTime <= p_ulTimeout && ulCurTime >= ulStartTime))
			{
				if (0 == ReadFile (p_pstComPort->hComPort, p_pucData, p_iSize, (unsigned long *) p_piLen, 0))
				{
					ucRet = 0x04;
					goto CleanUp;
				}

				if (0 < *p_piLen)
				{
					break;
				}

				SLEEP (1);
				GET_TICKCOUNT (ulCurTime);
			}
		}
		else
		{
			if (0 == ReadFile (p_pstComPort->hComPort, p_pucData, p_iSize, (unsigned long *) p_piLen, 0))
			{
				ucRet = 0x04;
				goto CleanUp;
			}

			#if defined (DEBUG)

				for (iIndex = 0; iIndex < *p_piLen; iIndex++)
				{
					printf ("%c", *(p_pucData + iIndex));
				}

				fflush (stdout);

			#endif
		}

		ucRet = 0x00;

	#elif defined (LINUX) || defined (UCLINUX)

		FD_ZERO (&stReadFds);
		FD_ZERO (&stExcpFds);

		FD_SET (p_pstComPort->hComPort, &stReadFds);
		FD_SET (p_pstComPort->hComPort, &stExcpFds);
		
		if (0xFFFFFFFF != p_ulTimeout)
		{
			// Set read timeout
			stTime.tv_sec = (time_t) (p_ulTimeout / 1000);
			stTime.tv_usec = (long) (1000 * (p_ulTimeout % 1000));

			iRet = select (p_pstComPort->hComPort + 1, &stReadFds, 0, &stExcpFds, &stTime);

			if (0 == iRet)
			{
				ucRet = 0;	// No data in Com port buffer
				goto CleanUp;
			}
			else if (0 < iRet)
			{
				if (0 != FD_ISSET (p_pstComPort->hComPort, &stExcpFds))
				{
					ucRet = 0x06;	// Error during checking recv status
					goto CleanUp;
				}

				if (0 == FD_ISSET (p_pstComPort->hComPort, &stReadFds))
				{
					ucRet = 0x00;	// No incoming data
					goto CleanUp;
				}
			}
			else
			{
				if (EINTR == errno)
				{
					ucRet = 0x00;	// Interrupted signal catched
				}
				else
				{
					ucRet = 0x07;	// Error during checking recv status
				}

				goto CleanUp;
			}
		}
		
		// Get data from Com port buffer
		iRet = read (p_pstComPort->hComPort, p_pucData, p_iSize);

		if (0 > iRet)
		{
			if (EINTR == errno)
			{
				ucRet = 0x00;	// Interrupted signal catched
			}
			else
			{
				ucRet = 0x04;	// Failed to read Com port
			}
		}
		else
		{
			*p_piLen = iRet;

			#if defined (DEBUG)

				for (iIndex = 0; iIndex < iRet; iIndex++)
				{
					printf ("%c", *(p_pucData + iIndex));
				}

				fflush (stdout);

			#endif

			ucRet = 0;
		}

	#endif

CleanUp:

	return ucRet;
}
