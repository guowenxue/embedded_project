#ifndef __LINK_LIST_H
#define __LINK_LIST_H


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


struct __LINK_NODE;

typedef struct __TEST
{
   struct __LINK_NODE          *node;
   int                          data;
} TEST;

typedef struct __LINK_NODE
{
   int                     data;
   struct __LINK_NODE      *next;
   TEST                    *test;
} LINK_NODE;

LINK_NODE    *link_create(void);
unsigned char link_insert(LINK_NODE *head, int pos, int data);
unsigned char link_del(LINK_NODE *head, int pos);
void link_revers(LINK_NODE *head);
void link_destroy(LINK_NODE *head);
void link_traverse(LINK_NODE *head);

#endif
