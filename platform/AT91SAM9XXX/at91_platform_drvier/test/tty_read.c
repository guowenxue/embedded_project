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

#define BUF_SIZE      80

/********************************************************************************
 *  Description:
 *   Input Args:
 *  Output Args:
 * Return Value:
 ********************************************************************************/
int main(int argc, char **argv)
{
    int fd = -1;
    int ret, i;
    char  buf[BUF_SIZE];
    int count = 0;

    fd = open("/dev/gstty0", O_RDWR);
    if (fd < 0)
    {
        perror("Open gstty failure\n");
        return -1;
    }

    while (1)
    { 
        memset(buf, 0, BUF_SIZE);
        ret = read(fd, buf, BUF_SIZE); 
        if(ret < 0) 
        {
            perror("Write falure"); 
        }

        if(ret>0)
        { 
            for(i=0; i<ret; i++) 
            { 
                printf("%c", buf[i]); 
            } 
            count ++;

            if(80==count)
            { 
                printf("\n"); 
                count=0;
            }
        }

    }

    close(fd);
    return 0;
}                               /* ----- End of main() ----- */
