
#include    "comport.h"

static inline void msleep(unsigned long ms)
{
    ms *= 1000;
    usleep(ms);
}

void init_default_comport(COM_PORT * comport, char *dev_name)
{
    memset(comport, 0, sizeof(COM_PORT));
    strncpy(comport->dev_name, dev_name, DEVNAME_LEN);
    comport->baudrate = 115200;
    comport->databit = 8;
    comport->parity = 0;
    comport->stopbit = 1;
    comport->flowctrl = 0;
    comport->is_connted = 0;
    comport->frag_size = 64;

    return;
}

void comport_init(COM_PORT * comport, char *dev_name, long baudrate,
                  int databit, int parity, int stopbit, int flowctrl)
{
    memset(comport, 0, sizeof(COM_PORT));
    strncpy(comport->dev_name, dev_name, DEVNAME_LEN);
    comport->baudrate = baudrate;
    comport->databit = databit;
    comport->stopbit = stopbit;
    comport->parity = parity;
    comport->flowctrl = flowctrl;
    comport->is_connted = 0;
    comport->frag_size = 64;

    return;
}

#if 0
void disp_setting(COM_PORT * comport)
{
#ifdef DEBUG
    dbg_print("Baudrate:\t\t\t\"%ld\"\n", comport->baudrate);
    dbg_print("DataBit:\t\t\t\"%d\"\n", comport->databit);
    switch (comport->parity)
    {
      case 0:
          dbg_print("Parity:\t\t\t\t\"N\"\n");
          break;
      case 1:
          dbg_print("Parity:\t\t\t\t\"O\"\n");
          break;
      case 2:
          dbg_print("Parity:\t\t\t\t\"E\"\n");
          break;
      case 3:
          dbg_print("Parity:\t\t\t\t\"S\"\n");
          break;
    }
    dbg_print("StopBit:\t\t\t\"%ld\"\n", (long int)comport->stopbit);
    switch (comport->flowctrl)
    {
      case 0:
          dbg_print("FlowCtrl:\t\t\t\"N\"\n");
          break;
      case 1:
          dbg_print("FlowCtrl:\t\t\t\"S\"\n");
          break;
      case 2:
          dbg_print("FlowCtrl:\t\t\t\"H\"\n");
          break;
      case 3:
          dbg_print("FlowCtrl:\t\t\t\"B\"\n");
          break;
    }
    dbg_print("\n");
#endif
    return;
}
#endif

// set:  8N1N
void set_setting(char *set, COM_PORT * comport)
{
//  dbg_print("Come into %s() with %s\n", __FUNCTION__, set);
    switch (*set)               // data bit
    {
      case '7':
          comport->databit = 7;
          break;
      default:
          comport->databit = 8;
          break;
    }

    switch (*(set + 1))         // parity
    {
      case 'O':
          comport->parity = 1;
          break;
      case 'E':
          comport->parity = 2;
          break;
      case 'S':
          comport->parity = 3;
          break;
      default:
          comport->parity = 0;
          break;
    }

    switch (*(set + 2))         // stop bit
    {
      case '0':
          comport->stopbit = 0;
          break;
      default:
          comport->stopbit = 1;
          break;
    }

    switch (*(set + 3))         // flow control
    {
      case 'S':
          comport->flowctrl = 1;
          break;
      case 'H':
          comport->flowctrl = 2;
          break;
      case 'B':
          comport->flowctrl = 3;
          break;
      default:
          comport->flowctrl = 0;
          break;
    }
#if 0
    dbg_print("Set StopBit as:\t\t\t\"%c\"\n", *set);
    dbg_print("Set Partity as:\t\t\t\"%c\"\n", *(set + 1));
    dbg_print("Set Stopbit as:\t\t\t\"%c\"\n", *(set + 2));
    dbg_print("Set flow control as:\t\t\"%c\"\n", *(set + 3));
#endif
}

void comport_close(COM_PORT * comport)
{
    if (0 != comport->fd)
    {
        COM_PRINT("Close \"%s\" now.\n", comport->dev_name);
        close(comport->fd);
    }
    comport->is_connted = 0x00;
    comport->fd = 0x00;
}

void comport_term(COM_PORT * comport)
{
    if (0 != comport->fd)
    {
        comport_close(comport);
    }
    memset(comport, 0x00, sizeof(COM_PORT));
    return;
}

unsigned char comport_open(COM_PORT * comport)
{
    unsigned char ucRet;
    struct termios old_cfg, new_cfg;
    int old_flags;
    long tmp;

    comport_close(comport);

    comport->fd = open(comport->dev_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (comport->fd < 0)
    {
        ucRet = 5;
        goto CleanUp;
    }

    if ((-1 != (old_flags = fcntl(comport->fd, F_GETFL, 0)))
        && (-1 != fcntl(comport->fd, F_SETFL, old_flags & ~O_NONBLOCK)))
    {
        // Flush input and output
        if (-1 == tcflush(comport->fd, TCIOFLUSH))
        {
            ucRet = 0x06;
            goto CleanUp;
        }
    }
    else                        // Failure
    {
        ucRet = 0x06;
        goto CleanUp;
    }

    if (0 != tcgetattr(comport->fd, &old_cfg))
    {
        ucRet = 0x07;           // Failed to get Com settings  
        goto CleanUp;
    }

    memset(&new_cfg, 0, sizeof(new_cfg));

    /*=====================================*/
    /*       Configure comport         */
    /*=====================================*/

    new_cfg.c_cflag &= ~CSIZE;
    new_cfg.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    new_cfg.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    new_cfg.c_oflag &= ~(OPOST);

    /* Set the data bit */
    switch (comport->databit)
    {
      case 0x07:
          new_cfg.c_cflag |= CS7;
          break;
      case 0x06:
          new_cfg.c_cflag |= CS6;
          break;
      case 0x05:
          new_cfg.c_cflag |= CS5;
          break;
      default:
          new_cfg.c_cflag |= CS8;
          break;
    }

    /* Set the parity */
    switch (comport->parity)
    {
      case 0x01:               // Odd  
          new_cfg.c_cflag |= (PARENB | PARODD);
          new_cfg.c_cflag |= (INPCK | ISTRIP);
          break;
      case 0x02:               // Even 
          new_cfg.c_cflag |= PARENB;
          new_cfg.c_cflag &= ~PARODD;;
          new_cfg.c_cflag |= (INPCK | ISTRIP);
          break;
      case 0x03:
          new_cfg.c_cflag &= ~PARENB;
          new_cfg.c_cflag &= ~CSTOPB;
          break;
      default:
          new_cfg.c_cflag &= ~PARENB;
    }

    /* Set Stop bit */
    if (0x01 != comport->stopbit)
    {
        new_cfg.c_cflag |= CSTOPB;
    }
    else
    {
        new_cfg.c_cflag &= ~CSTOPB;
    }

    /* Set flow control */
    switch (comport->flowctrl)
    {
      case 1:                  // Software control 
      case 3:
          new_cfg.c_cflag &= ~(CRTSCTS);
          new_cfg.c_iflag |= (IXON | IXOFF);
          break;
      case 2:                  // Hardware control
          new_cfg.c_cflag |= CRTSCTS;   // Also called CRTSCTS
          new_cfg.c_iflag &= ~(IXON | IXOFF);
          break;
      default:                 // NONE
          new_cfg.c_cflag &= ~(CRTSCTS);
          new_cfg.c_iflag &= ~(IXON | IXOFF);
          break;
    }

    /* Set baudrate */
    switch (comport->baudrate)
    {
      case 115200:
          tmp = B115200;
          break;
      case 57600:
          tmp = B57600;
          break;
      case 38400:
          tmp = B38400;
          break;
      case 19200:
          tmp = B19200;
          break;
      case 9600:
          tmp = B9600;
          break;
      case 4800:
          tmp = B4800;
          break;
      case 2400:
          tmp = B2400;
          break;
      case 1800:
          tmp = B1800;
          break;
      case 1200:
          tmp = B1200;
          break;
      case 600:
          tmp = B600;
          break;
      case 300:
          tmp = B300;
          break;
      case 200:
          tmp = B200;
          break;
      case 150:
          tmp = B150;
          break;
      case 134:
          tmp = B134;
          break;
      case 110:
          tmp = B110;
          break;
      case 75:
          tmp = B75;
          break;
      case 50:
          tmp = B50;
          break;
      default:
          tmp = B115200;
    }
    cfsetispeed(&new_cfg, tmp);
    cfsetispeed(&new_cfg, tmp);

    /* Set the Com port timeout settings */
    new_cfg.c_cc[VMIN] = 0;
    new_cfg.c_cc[VTIME] = 0;

    tcflush(comport->fd, TCIFLUSH);
    if (0 != tcsetattr(comport->fd, TCSANOW, &new_cfg))
    {
        ucRet = 0x08;           // Failed to set device com port settings 
        goto CleanUp;
    }

    comport->is_connted = 0x01;
    ucRet = 0x00;

  CleanUp:
    return ucRet;
}

unsigned char comport_recv(COM_PORT * comport, char *buf,
                           int buf_size, unsigned int *recv_bytes, unsigned long timeout)
{
    unsigned char ucRet;        // Function return value
    int iRet;
    fd_set stReadFds, stExcpFds;
    struct timeval stTime;

    if (NULL == buf || 0 >= buf_size || NULL == recv_bytes)
    {
        COM_PRINT("%s() usage error.\n", __FUNCTION__);
        ucRet = 0x02;
        goto CleanUp;
    }
    else
        *recv_bytes = 0;

    if (0x01 != comport->is_connted)
    {
        COM_PRINT("%s() comport not connected.\n", __FUNCTION__);
        ucRet = 0x03;
        goto CleanUp;
    }

    FD_ZERO(&stReadFds);
    FD_ZERO(&stExcpFds);
    FD_SET(comport->fd, &stReadFds);
    FD_SET(comport->fd, &stExcpFds);

    if (0xFFFFFFFF != timeout)
    {
        stTime.tv_sec = (time_t) (timeout / 1000);
        stTime.tv_usec = (long)(1000 * (timeout % 1000));

        iRet = select(comport->fd + 1, &stReadFds, 0, &stExcpFds, &stTime);
        if (0 == iRet)
        {
            ucRet = 0;          // No data in Com port buffer
            goto CleanUp;
        }
        else if (0 < iRet)
        {
            if (0 != FD_ISSET(comport->fd, &stExcpFds))
            {
                ucRet = 0x06;   // Error during checking recv status  
                COM_PRINT("Error checking recv status.\n");
                goto CleanUp;
            }

            if (0 == FD_ISSET(comport->fd, &stReadFds))
            {
                ucRet = 0x00;   // No incoming data 
                COM_PRINT("No incoming data.\n");
                goto CleanUp;
            }
        }
        else
        {
            if (EINTR == errno)
            {
                COM_PRINT("catch interrupt signal.\n");
                ucRet = 0x00;   // Interrupted signal catched
            }
            else
            {
                COM_PRINT("Check recv status failure.\n");
                ucRet = 0x07;   // Error during checking recv status
            }

            goto CleanUp;
        }
    }
    msleep(100);                /*Sleep 100 ms */
    // Get data from Com port
    iRet = read(comport->fd, buf, buf_size);
    if (0 > iRet)
    {
        if (EINTR == errno)
            ucRet = 0x00;       // Interrupted signal catched
        else
            ucRet = 0x04;       // Failed to read Com port
    }
    else
    {
        *recv_bytes = iRet;
        ucRet = 0x00;
    }

  CleanUp:
    return ucRet;

}

unsigned char comport_send(COM_PORT * comport, char *buf, int send_bytes)
{
    unsigned char ucRet = 0x00;
   char  *ptr, *end;
    int send = 0;

    if (NULL == buf || 0 >= send_bytes)
    {
        COM_PRINT("%s() Usage error.\n", __FUNCTION__);
        ucRet = 0x01;
        goto CleanUp;
    }

    if (0x01 != comport->is_connted)    // Comport not opened ?
    {
        ucRet = 0x02;
        COM_PRINT("Serail not connected.\n");
        goto CleanUp;
    }
    //printf("Send %s with %d bytes.\n", buf, send_bytes);

    // Large data, then slice them and send
    if (comport->frag_size < send_bytes)
    {
        ptr = buf;
        end = buf + send_bytes;

        do
        {
            // Large than frag_size
            if (comport->frag_size < (end - ptr))
            {
                send = write(comport->fd, ptr, comport->frag_size);
                if (0 >= send || comport->frag_size != send)
                {
                    ucRet = 0x03;
                    goto CleanUp;
                }
                ptr += comport->frag_size;
            }
            else                // Less than frag_size, maybe last fragmention.
            {
                send = write(comport->fd, ptr, (end - ptr));
                if (0 >= send || (end - ptr) != send)
                {
                    ucRet = 0x03;
                    goto CleanUp;
                }
                ptr += (end - ptr);
            }
        }
        while (ptr < end);
    }
    else                        // The send data is not large than a fragmention.
    {
        send = write(comport->fd, buf, send_bytes);
        if (0 >= send || send_bytes != send)
        {
            ucRet = 0x03;
            goto CleanUp;
        }
    }

  CleanUp:
    return ucRet;
}
