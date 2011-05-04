#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
/** 
 * container_of - cast a member of a structure out to the containing structure 
 * @ptr:        the pointer to the member. 
 * @type:       the type of the container struct this is embedded in. 
 * @member:     the name of the member within the struct. 
 * 
 */ 
#define container_of(ptr, type, member) ({ \
                const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
                (type *)( (char *)__mptr - offsetof(type,member) );})


typedef struct __QCBITMAP
{
        /*This Item is for LED,Beep,Card EEPROM,Backplane EEPROM,USB,SD Card,RTC test item*/
        unsigned int    uiMisc; 
        
        /*This is for PSTN test item*/ 
        unsigned int    uiPstn; 
        
        /*This is for GPRS,ETHERNET,UART communication test item*/ 
        unsigned int    uiComm; 
        
       /*This is for Optimus test item*/ 
        unsigned int    uiOptm;
} QCBITMAP;




int main(void)
{
   QCBITMAP           stQcBitmap;
   QCBITMAP           *pstQcBitmap = NULL;
   unsigned int       *p_uiPstn = &(stQcBitmap.uiPstn);


   memset(&stQcBitmap, 0, sizeof(QCBITMAP));
   *(p_uiPstn) = 0x5a5a;


   pstQcBitmap =  container_of(p_uiPstn, QCBITMAP, uiPstn);

   printf("uiMisc: 0x%0x\n", pstQcBitmap->uiMisc);
   printf("uiPstn: 0x%0x\n", pstQcBitmap->uiPstn);
   printf("uiComm: 0x%0x\n", pstQcBitmap->uiComm);
   printf("uiOptm: 0x%0x\n", pstQcBitmap->uiOptm);

   return 0;
}


