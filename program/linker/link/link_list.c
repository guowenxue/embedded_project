#include "link_list.h"
#include "linker_errno.h"

LINK_NODE *link_create(void)
{
   LINK_NODE         *head=NULL;

   head = (LINK_NODE *)malloc(sizeof(LINK_NODE));
   head->next = NULL; 
   head->data = -1;
   return head;
}

unsigned char link_insert(LINK_NODE *head, int pos, int data)
{
   int               i=1;
   unsigned char     retVal = COMMON_OK;
   LINK_NODE         *ptr = head;
   LINK_NODE         *node = NULL;

   while(NULL != ptr)
   {
      if(i == pos)  
      {
           if(NULL ==(node=(LINK_NODE *)malloc(sizeof(LINK_NODE))) )
           {
                retVal = MEM_ALLOC_FAILURE; 
                goto CleanUp;
           }
           node->data = data;
           ptr->next = node;
           break;
      }
      else
      {
           ptr = ptr->next; 
           i++;
      }
   }

CleanUp:
   return retVal;
}

unsigned char link_del(LINK_NODE *head, int pos)
{
   return COMMON_OK;
}

void link_revers(LINK_NODE *head)
{
   LINK_NODE        *revsd_ptr;  /* Last one time reversed linker pointer*/
   LINK_NODE        *cur_ptr;    /* Currently need reverse node pointer*/
   LINK_NODE        *next_ptr;   /* Next need revers node pointer*/

   revsd_ptr = NULL;
   cur_ptr = head->next;
   while( cur_ptr != NULL)
   {
       next_ptr = cur_ptr->next;
       cur_ptr->next = revsd_ptr;
       revsd_ptr =  cur_ptr;
       cur_ptr = next_ptr;
   }

   head->next = revsd_ptr;
}

/*Free all the node include head node*/
void link_destroy(LINK_NODE *head)
{
   LINK_NODE          *ptr = NULL;

   while( head )
   {
          ptr = head;
          head = head->next;
          printf("Free node: %d\n", ptr->data);
          free(ptr); 
   }

}

void link_traverse(LINK_NODE *head)
{
  LINK_NODE      *ptr = head->next;
  int            i = 1;

  while(NULL != ptr)
  {
      printf("node[%d]->data=%d\n", i, ptr->data); 
      ptr = ptr->next;
      i++;
  }
}

