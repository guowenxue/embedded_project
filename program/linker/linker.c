#include <stdio.h>
#include "linker_errno.h"
#include "link/link_list.h"

int main(int argc, char **argv)
{
      unsigned char       retVal;
      int                 i;
      LINK_NODE           *head = NULL;


      if( NULL == (head=link_create()) )
              return -1;

      for(i=1; i<5; i++)
      {
           retVal = link_insert(head, i, i);
           if(COMMON_OK != retVal) 
           {
                 printf("Insert node[%d] failure, retVal = 0x%02x\n", i, retVal); 
                 break;
           }
      }
      link_traverse(head);

      link_revers(head);
      printf("After revers...........................................................\n");
      link_traverse(head);

      printf("Destroy the linker now.\n");
      link_destroy(head);

      return 0;
}

