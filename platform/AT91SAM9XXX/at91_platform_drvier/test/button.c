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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MODE_POLL     0x01
#define MODE_NORMAL   0x02

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
    unsigned int status;
    unsigned mode = MODE_NORMAL;

    printf("Pass a argument will use POLL mode to test the button driver\n");
    if (argc > 1)
    {
        mode = MODE_POLL;
    }

    fd = open("/dev/button", O_RDONLY);
    if (fd < 0)
    {
        perror("Open button failure\n");
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
                printf("Select Buttons Device Faild!\n");
                return -1;
            }

            if (ret == 0)
            {
                printf("Read Buttons Device Timeout!\n");
            }
            else if (FD_ISSET(fd, &rds))
            {
                ret = read(fd, &status, sizeof(status));    //读设备
                if (ret != sizeof(status))
                {
                    if (errno != EAGAIN)
                    {
                        printf("Read Button Device Faild!\n");
                    }
                    continue;
                }
                else
                    printf("status %x\n", status);
            }
        }
        else
        {
            ret = read(fd, &status, sizeof(status));    //读设备
            if (ret != sizeof(status))
            {
                printf("ret=%d\n", ret);
            }
            else
            {
                printf("button status: %d\n", status);
            }

            sleep(1);
        }
    }

    close(fd);
    return 0;
}                               /* ----- End of main() ----- */
