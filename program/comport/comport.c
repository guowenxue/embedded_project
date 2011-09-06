#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>

#include "CComPort.h"
#include "version.h"

#define	SYS_NAME_NAME		"comport"

unsigned char		g_ucProcToken,
					g_ucCtrlZ;

void syslog (int type, char* msg, ...)
{
	va_list argptr;

	va_start (argptr, msg);
	vprintf (msg, argptr);
	va_end (argptr);

	printf ("\n");
}

void usage ()
{
	syslog (1, "%s version %d.%d.%d Build %04d", SYS_NAME_NAME, MAJOR, MINOR, REVER, SVNVER);
	syslog (1, "Usage: comport -d <device> [-b <baudrate>][-s <settings>][-sc16][-ioctl <driver port> <cmd> <arg>][--help][--version]");
	syslog (1, "  -d         device name");
	syslog (1, "  -b         device baudrate (115200, 57600, 19200, 9600), default is 115200");
	syslog (1, "  -s         device settings (data bits, parity, stop bits, flow control), default is 8N1N");
	syslog (1, "              - data bits: 8, 7");
	syslog (1, "              - parity: N=None, O=Odd, E=Even, S=Space");
	syslog (1, "              - stop bits: 1, 0");
	syslog (1, "              - flow control: N=None, H=Hardware, S=Software, B=Both");
	syslog (1, "  -hex       display received data in hex format");
	syslog (1, "  -sc16      special configure for SC16 driver");
	syslog (1, "  -ioctl     ioctl system call (cmd & arg only support int)");
	syslog (1, "  --help     help info");
	syslog (1, "  --version  version number");
}

void signal_handler (int i_sig)
{
	if (SIGTERM == i_sig || SIGINT == i_sig)
	{
		g_ucProcToken = 0x00;
	}
	else if (20 == i_sig)
	{
		g_ucCtrlZ = 0x01;
	}
}

int kbhit ()
{
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &fds);
}

void nonblock ()
{
    struct termios ttystate;

    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);

    //turn off canonical mode
    ttystate.c_lflag &= ~ICANON;
    //minimum of number input read.
    ttystate.c_cc[VMIN] = 1;

    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int main (int argc, char *argv [])
{
	int					iRet			= 1,
						iIndex,
						iDataBits		= 8,
						iParity			= 0,
						iStopBits		= 1,
						iFlowCtrl		= 0,
						iRecv			= 0,
						iDisplayMode	= 0,
						iCmd			= 0,
						iArg			= 0;
	
	long				lBaudrate		= 115200;
	
	char				acDev [128]		= { 0 },
						acCfg [5]		= { 0 },
						acDrv [128]		= { 0 };
	
	unsigned char		ucRet,
						ucDevType		= 0x00,
						aucSC16 [4]		= { 0 },
						aucBuf [512]	= { 0 };
	
	COM_PORT			stComPort,
						stDrv;
	
    struct sigaction	sigact;

	g_ucProcToken = 0x01;
	g_ucCtrlZ = 0x00;
	
	for (iIndex = 1; iIndex < argc; iIndex++)
	{
		if (0 == strncmp (argv [iIndex], "--help", strlen ("--help")))
		{
			usage ();
			iRet = 0;
			goto CleanUp;
		}
		else if (0 == strncmp (argv [iIndex], "--version", strlen ("--version")))
		{
			syslog (1, "%s version %d.%d.%d Build %04d", SYS_NAME_NAME, MAJOR, MINOR, REVER, SVNVER);
			syslog (1, "Copyright (C) 2011 by GHL Systems Berhad.");
			iRet = 0;
			goto CleanUp;
		}
		else if (2 <= (argc - iIndex) && 0 == strncmp (argv [iIndex], "-d", strlen ("-d")) && NULL != argv [iIndex + 1])
		{
			strncpy (acDev, argv [iIndex + 1], sizeof (acDev) - 1);
			iIndex++;
		}
		else if (4 <= (argc - iIndex) && 0 == strncmp (argv [iIndex], "-ioctl", strlen ("-ioctl")) && NULL != argv [iIndex + 1] && NULL != argv [iIndex + 2] && NULL != argv [iIndex + 3])
		{
			strncpy (acDrv, argv [iIndex + 1], sizeof (acDrv) - 1);
			iCmd = atoi (argv [iIndex + 2]);
			iArg = atoi (argv [iIndex + 3]);
			iIndex += 3;
		}
		else if (2 <= (argc - iIndex) && 0 == strncmp (argv [iIndex], "-b", strlen ("-b")) && NULL != argv [iIndex + 1])
		{
			lBaudrate = atol (argv [iIndex + 1]);
			iIndex++;
		}
		else if (0 == strncmp (argv [iIndex], "-hex", strlen ("-hex")))
		{
			iDisplayMode = 1;
		}
		else if (2 <= (argc - iIndex) && 0 == strncmp (argv [iIndex], "-s", strlen ("-s")) && NULL != argv [iIndex + 1])
		{
			if (4 == strlen (argv [iIndex + 1]))
			{
				strncpy (acCfg, argv [iIndex + 1], sizeof (acCfg) - 1);
				
				// Data bits
				switch (acCfg [0])
				{
				case '8': 	iDataBits = 8; break;
				case '7': 	iDataBits = 7; break;
				}

				// Parity
				switch (acCfg [1])
				{
				case 'N': 	iParity = 0; break;
				case 'O': 	iParity = 1; break;
				case 'E': 	iParity = 2; break;
				case 'S': 	iParity = 3; break;
				}

				// Stop bits
				switch (acCfg [2])
				{
				case '1': 	iStopBits = 1; break;
				case '0': 	iStopBits = 0; break;
				}

				// Flow control
				switch (acCfg [3])
				{
				case 'N': 	iFlowCtrl = 0; break;
				case 'S': 	iFlowCtrl = 1; break;
				case 'H': 	iFlowCtrl = 2; break;
				case 'B': 	iFlowCtrl = 3; break;
				}
			}

			iIndex++;
		}
		else if (0 == strncmp (argv [iIndex], "-sc16", strlen ("-sc16")))
		{
			ucDevType = 0x01;
		}
		else
		{
			syslog (1, "unknown parameter: %s", argv [iIndex]);
			usage ();
			iRet = 1;
			goto CleanUp;
		}
	}

	if (0 != acDrv [0])
	{
		comport_init (&stDrv, acDrv, 0, 0, 0, 0, 0, 0, 0, 0);

		ucRet = comport_open (&stDrv);

		if (0x00 == ucRet)
		{
			iRet = ioctl (stDrv.hComPort, iCmd, iArg);
			comport_close (&stDrv);
			
			syslog (1, "ioctl (%s, %d, %d) returned %d", acDrv, iCmd, iArg, iRet);
			return 0;
		}
		else
		{
			syslog (1, "failed to open %s. RetCode [0x%02X]", acDrv, ucRet);
		}
	}

	if (0 == acDev [0] || -1 == lBaudrate || -1 == iDataBits || -1 == iParity || -1 == iStopBits || -1 == iFlowCtrl)
	{
		usage ();
		iRet = 1;
		goto CleanUp;
	}
	
	// Process level signal handler
	sigemptyset (&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = signal_handler;

    sigaction (SIGTERM, &sigact, NULL);	// catch terminate signal
    sigaction (SIGINT, &sigact, NULL);	// catch interrupt signal
    sigaction (SIGSEGV, &sigact, NULL);	// catch segmentation faults
	sigaction (SIGTSTP, &sigact, NULL);	// catch ctrl+Z
    sigaction (SIGSTOP, &sigact, NULL);	// catch ctrl+Z

	// Open Com port
	if (0x00 == ucDevType)
	{
		comport_init (&stComPort, acDev, lBaudrate, (unsigned char) iDataBits, (unsigned char) iStopBits, (unsigned char) iParity, (unsigned char) iFlowCtrl, 64, 0, 0);

		ucRet = comport_open (&stComPort);

		if (0x00 != ucRet)
		{
			syslog (1, "Failed to open %s with baudrate %ld, %s. RetCode [0x%02X]", acDev, lBaudrate, acCfg, ucRet);
			iRet = 1;
			goto CleanUp;
		}
	}
	else if (0x01 == ucDevType)
	{
		comport_init (&stComPort, acDev, 0, 0, 0, 0, 0, 0, 0, 0);

		ucRet = comport_open (&stComPort);
		
		if (0x00 != ucRet)
		{
			syslog (1, "Failed to open %s with baudrate %ld, %s. RetCode [0x%02X]", acDev, lBaudrate, acCfg, ucRet);
			iRet = 1;
			goto CleanUp;
		}

		// Configure the sc16 driver settings
		switch (lBaudrate)
		{
		case 2400:		aucSC16 [0] = 0xC0; break;
		case 4800:		aucSC16 [0] = 0x60; break;
		case 9600:		aucSC16 [0] = 0x30; break;
		case 19200:		aucSC16 [0] = 0x18; break;
		case 38400:		aucSC16 [0] = 0x0C; break;
		case 57600:		aucSC16 [0] = 0x08; break;
		case 76800:		aucSC16 [0] = 0x06; break;
		case 115200:	aucSC16 [0] = 0x04; break;
		default:		aucSC16 [0] = 0x08; break;
		}

		switch (iParity)
		{
		case 0:		aucSC16 [1] = 0x00; break;	// None
		case 1:		aucSC16 [1] = 0x01; break;	// Odd
		case 2:		aucSC16 [1] = 0x03; break;	// Even
		case 3:		aucSC16 [1] = 0x05; break;	// Space
		default:	aucSC16 [1] = 0x00; break;
		}

		switch (iDataBits)
		{
		case 8:		aucSC16 [2] = 0x08; break;
		case 7:		aucSC16 [2] = 0x07; break;
		case 6:		aucSC16 [2] = 0x06; break;
		case 5:		aucSC16 [2] = 0x05; break;
		default:	aucSC16 [2] = 0x08; break;
		}

		switch (iStopBits)
		{
		case 1:		aucSC16 [3] = 0x10; break;
		case 2:		aucSC16 [3] = 0x20; break;
		default:	aucSC16 [3] = 0x10; break;
		}

		iRet = ioctl (stComPort.hComPort, 0x14, aucSC16);

		if (0 != iRet)
		{
			syslog (1, "Failed to configure %s with baudrate %ld, %s. RetCode [%d]", acDev, lBaudrate, acCfg, iRet);
			iRet = 2;
			goto CleanUp;
		}
	}
	
	
	nonblock ();
	
	while (0x01 == g_ucProcToken)
	{
		ucRet = comport_recv (&stComPort, aucBuf, sizeof (aucBuf) - 1, &iRecv, 10);
		
		if (0x00 == ucRet && 0 < iRecv)
		{
			for (iIndex = 0; iIndex < iRecv; iIndex++)
			{
				if (0 == iDisplayMode)
					printf ("%c", aucBuf [iIndex]);
				else
					printf ("%02X ", aucBuf [iIndex]);
			}
			
			fflush (stdout);
		}
		
		if (0 != kbhit ())
		{
			iRecv = fgetc (stdin);
			
			if (10 == iRecv)
			{
				aucBuf [0] = 13;
			}
			else
			{
				aucBuf [0] = iRecv;
			}

			comport_send (&stComPort, aucBuf, 1);
		}
		else if (0x00 != g_ucCtrlZ)
		{
			g_ucCtrlZ = 0x00;
			
			aucBuf [0] = 0x1A;
			comport_send (&stComPort, aucBuf, 1);
		}
	}
	
	comport_close (&stComPort);
	
	iRet = 0;
	
CleanUp:
	comport_term (&stComPort);

	return iRet;
}
