/*********************************************************************************
 *      Copyright:  (C) 2011 R&D of San Fran Electronics Co., LTD  
 *                  All rights reserved.
 *
 *       Filename:  adc_read.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(11/23/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "11/23/2011 09:53:01 AM"
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
#include <sys/ioctl.h>
#include <signal.h>

#define DATA_LEN       10

/********************************************************************************
 *  Description:
 *   Input Args:
 *  Output Args:
 * Return Value:
 ********************************************************************************/
int main (int argc, char **argv)
{
    int fd = -1;
    int len, i;
    int sample_data[DATA_LEN]={0};
    char *dev;
    int count;

    if(argc != 3)
    {
        printf("Usage: %s [dev_name] [count]\n", argv[0]);
        return 0;
    }

    dev = argv[1];
    count = atoi(argv[2]);

    if( (fd=open(dev, O_RDONLY)) < 0)
    {
        printf("Open %s failure: %s\n", dev, strerror(errno));
        return -1;
    }

    len = read(fd, sample_data, count*sizeof(int));
    printf("Read %d bytes data:\n", len);
    for(i=0; i<count; i++)
    {
        printf("%d ", sample_data[i]);
    }
    printf("\n");

    close(fd);
    return 0;
} /* ----- End of main() ----- */

