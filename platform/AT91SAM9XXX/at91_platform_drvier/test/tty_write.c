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

#define BUF_SIZE      128

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
    unsigned long long count = 0;
    char  buf[BUF_SIZE];

    fd = open("/dev/gstty0", O_RDWR);
    if (fd < 0)
    {
        perror("Open gstty failure\n");
        return -1;
    }

    printf("Initialize data:\n");
    for(i=0; i<BUF_SIZE; i++)
    {
        buf[i] = i%10;
        printf("%d", buf[i]);
    }
    printf("\n");
    while (1)
   // for(i=0; i<3; i++) 
    { 

        ret = write(fd, buf, BUF_SIZE); 
     //   ret = write(fd, buf, 2); 
        if(ret < 0) 
            perror("Write falure"); 

        count+=ret;
        printf("sending %llu bytes...\n", count);
        //usleep(50000);
    }

    close(fd);
    return 0;
}                               /* ----- End of main() ----- */
