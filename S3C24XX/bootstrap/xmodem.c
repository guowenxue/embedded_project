/********************************************************************************************
 *        File:  xmodem.c
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  Xmodem protocal used to download the second stage bootloader(launcher)
 *   ChangeLog:  1, Release initial version on "Tue Jul 12 16:43:18 CST 2011"
 *
 *******************************************************************************************/

#include <common.h>

#define XMODEM_SOH 0x01         /*Start Of Header, standard Xmodem */
#define XMODEM_STX 0x02         /*1K-Modem start Xmodem */
#define XMODEM_EOT 0x04         /*End Of Transmission */
#define XMODEM_ACK 0x06
#define XMODEM_NAK 0x15
#define XMODEM_CAN 0x18         /*Stop transmission */
#define XMODEM_EOF 0x1a         /*CTRL+Z EOF */

#define XMODEM_BLOCK_SIZE 128

static int xmodem_wait(void)
{
    long cnt = 0;

    while (!serial_is_recv_enable())
    {
        if (++cnt >= 2000000)
        {
            cnt = 0;
            serial_send_byte(XMODEM_NAK);
        }
    }
    return 0;
}

static int xmodem_read_block(unsigned char block_number, char *buf)
{
    unsigned char c, block_num, check_sum;
    int i;

    block_num = serial_recv_byte();
    if (block_num != block_number)
        return -1;

    block_num ^= serial_recv_byte();
    if (block_num != 0xff)
        return -1;

    check_sum = 0;
    for (i = 0; i < XMODEM_BLOCK_SIZE; i++)
    {
        c = serial_recv_byte();
        *(buf++) = c;
        check_sum += c;
    }

    check_sum ^= serial_recv_byte();
    if (check_sum)
        return -1;

    return i;
}

long xmodem_recv(char *buf)
{
    int r, receiving = 0;
    long size = 0;
    unsigned char c, block_number = 1;

    while (1)
    {
        if (!receiving)
            xmodem_wait();

        c = serial_recv_byte();

        switch (c)
        {
          case XMODEM_EOT:
              serial_send_byte(XMODEM_ACK);

              /*Remove the CMPEOF */
              long cnt = 0;
              if ((buf[-1] == XMODEM_EOF) && (buf[-2] == XMODEM_EOF) && (buf[-3] == XMODEM_EOF))
              {
                  while (size && buf[-cnt - 1] == XMODEM_EOF)
                      cnt++;
              }

              size -= cnt;
              goto RECV_OK;

          case XMODEM_CAN:
              return -1;

          case XMODEM_SOH:
              receiving++;
              r = xmodem_read_block(block_number, buf);
              if (r < 0)
              {
                  serial_send_byte(XMODEM_NAK);
              }
              else
              {
                  block_number++;
                  size += r;
                  buf += r;
                  serial_send_byte(XMODEM_ACK);
              }
              break;

          default:
              if (receiving)
                  return -1;
        }
    }

  RECV_OK:
    return size;
}
