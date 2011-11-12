/*********************************************************************************
 *      Copyright:  (C) 2011 R&D of San Fran Electronics Co., LTD  
 *                  All rights reserved.
 *
 *       Filename:  test.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(11/02/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "11/02/2011 10:55:45 AM"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MODE_POLL     0x01
#define MODE_NORMAL   0x02
#define BUF_SIZE      64

/********************************************************************************
 *  Description:
 *   Input Args:
 *  Output Args:
 * Return Value:
 ********************************************************************************/
int main(int argc, char **argv)
{
    int fd = -1;
    int ret;
    char  buf[BUF_SIZE];
    unsigned mode = MODE_NORMAL;

    printf("Pass a argument will use POLL mode to test the gstty driver\n");
    if (argc > 1)
    {
        mode = MODE_POLL;
    }

    fd = open("/dev/gstty0", O_RDWR);
    if (fd < 0)
    {
        perror("Open gstty failure\n");
        return -1;
    }

    while (1)
    {
        if (MODE_POLL == mode)
        {
            fd_set rds;

            FD_ZERO(&rds);
            FD_SET(fd, &rds);

            ret = select(fd + 1, &rds, NULL, NULL, NULL);
            if (ret < 0)
            {
                printf("Select gsttys Device Faild!\n");
                return -1;
            }

            if (ret == 0)
            {
                printf("Read gsttys Device Timeout!\n");
            }
            else if (FD_ISSET(fd, &rds))
            {
                ret = read(fd, buf, sizeof(BUF_SIZE));    //读设备
                if (ret <= 0)
                {
                    if (errno != EAGAIN)
                    {
                        printf("Read gstty Device Faild!\n");
                    }
                    continue;
                }
                else
                {
                    int i;
                    for (i=0; i<ret; i++)
                    printf("buf[%d] 0x%x\n", i, buf[i]);
                }
            }
        }
        else
        {
            ret = read(fd, buf, BUF_SIZE);    //读设备
            if (ret < 0)
            {
                perror("read failure");
            }
#if 0
            else
            {
                int i;
                for (i=0; i<ret; i++)
                  printf("buf[%d] %x\n", i, buf[i]);
            }
#endif
            if(ret>0)
            {
               ret = write(fd, buf, ret);
               if(ret < 0)
                   perror("Write falure");
            }

            memset(buf, 0x00, BUF_SIZE);

            //sleep(1);
        }
    }

    close(fd);
    return 0;
}                               /* ----- End of main() ----- */
