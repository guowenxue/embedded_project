/*********************************************************************************
 *      Copyright:  (C) 2011 R&D of San Fran Electronics Co., LTD  
 *                  All rights reserved.
 *
 *       Filename:  circle_buffer.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(11/14/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "11/14/2011 04:22:35 PM"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "circ_buf.h"

#define CIRC_BUF_SIZE 32
#define LEN           30

int produce_item(struct circ_buf *ring, char *data, int  count);
int consume_item(struct circ_buf *ring, char *buf, int  count);

/********************************************************************************
 *  Description:
 *   Input Args:
 *  Output Args:
 * Return Value:
 ********************************************************************************/
int main (int argc, char **argv)
{
    int i, len;
    struct circ_buf        tx_ring; 
    char   data[LEN];
    char   buf[LEN];

    memset(&tx_ring, 0, sizeof(struct circ_buf)); 
    tx_ring.buf = malloc(CIRC_BUF_SIZE); 
    if( NULL == tx_ring.buf )
    {
        printf("Allocate Ring buffer failure.\n");
        return -1;
    }

    memset(data, 0, sizeof(data));
    /* Prepare for the data */
    for(i=0; i<sizeof(data); i++)
    {
        data[i] = 30+i;
    }

    printf("CIRC_SPACE: %d\n", CIRC_SPACE(tx_ring.head, tx_ring.tail, CIRC_BUF_SIZE));
    printf("CIRC_SPACE_TO_END: %d\n", CIRC_SPACE_TO_END(tx_ring.head, tx_ring.tail, CIRC_BUF_SIZE));
    printf("CIRC_CNT: %d\n", CIRC_CNT(tx_ring.head, tx_ring.tail, CIRC_BUF_SIZE));
    printf("CIRC_CNT_TO_END: %d\n", CIRC_CNT_TO_END(tx_ring.head, tx_ring.tail, CIRC_BUF_SIZE));
    while(1)
    {
       produce_item(&tx_ring, data, sizeof(data));
       len = consume_item(&tx_ring, buf, sizeof(buf) );
       sleep(1);
    }

    return 0;
} /* ----- End of main() ----- */


int produce_item(struct circ_buf *ring, char *data, int  count)
{
    int len = 0;
    int left,i,size;
    int to_end_space=0;

    if ( (size=CIRC_SPACE(ring->head, ring->tail, CIRC_BUF_SIZE)) >= 1 )
    {
        left = len = count<=size ? count : size;
        to_end_space = CIRC_SPACE_TO_END(ring->head, ring->tail, CIRC_BUF_SIZE);

        if(left > to_end_space)
        { 
            memcpy(&(ring->buf[ring->head]), data, to_end_space); 
            for(i=0; i<to_end_space; i++) 
            { 
                printf("produec_item %02d bytes: ring->buf[%02d]=%d\n", to_end_space, ring->head+i, ring->buf[ring->head+i]); 
            }
            ring->head = (ring->head + to_end_space) & (CIRC_BUF_SIZE - 1); 
            left -= to_end_space; 
        }
        else
        { 
            to_end_space = 0;
        }

        memcpy(&(ring->buf[ring->head]), &data[to_end_space], left);
        for(i=0; i<left; i++)
        {
           printf("produec_item %02d bytes: ring->buf[%02d]=%d\n", left, ring->head+i, ring->buf[ring->head+i]);
        }
        ring->head = (ring->head + left) & (CIRC_BUF_SIZE - 1);
    }
    printf("-----------------------------------------------------------------------------------------------\n");

    return len;
}

int consume_item(struct circ_buf *ring, char *buf, int  count)
{
    int len = 0;
    int i, left, size;
    int to_end_space=0;

    if ( (size=CIRC_CNT(ring->head, ring->tail, CIRC_BUF_SIZE)) >= 1 )
    {
        left = len = count<=size ? count : size;
        to_end_space = CIRC_CNT_TO_END(ring->head, ring->tail, CIRC_BUF_SIZE);

        if(left > to_end_space)
        {
            memcpy(buf, &(ring->buf[ring->tail]), to_end_space);
            for(i=0; i<to_end_space; i++) 
            { 
                printf("consume_item %02d bytes: ring->buf[%02d]=%d\n", to_end_space, ring->tail+i, ring->buf[ring->tail+i]); 
            }
            ring->tail = (ring->tail + to_end_space) & (CIRC_BUF_SIZE - 1);
            left -= to_end_space;
        }
        else
        {
            to_end_space = 0;
        }

        memcpy(&buf[to_end_space], &(ring->buf[ring->tail]), left);
        for(i=0; i<left; i++)
        {
           printf("consume_item %02d bytes: ring->buf[%02d]=%d\n", left, ring->tail+i, ring->buf[ring->tail+i]);
        }
        ring->tail = (ring->tail + left) & (CIRC_BUF_SIZE - 1);
    }

    for(i=0; i<len; i++)
        printf("output_data %02d bytes: buf[%02d]=%d\n", len, i, buf[i]);

    printf("-----------------------------------------------------------------------------------------------\n");

    return len;
}

