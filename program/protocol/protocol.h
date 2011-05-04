#ifndef __PROTOCOL_H
#define __PROTOCOL_H

typedef unsigned char uchar;
extern uchar g_ucdebug;
#define dbg_print(format,args...)  if(0x00!=g_ucdebug)  { printf(format, ##args); }

typedef uchar (TEST_BODY)(uchar *pucParam, int iParmLen);
typedef struct __PROT_WORK
{
  uchar            ucCmd;
  TEST_BODY        *test_body; 
} PROT_WORK;

enum
{
   CMD_NONE,
   CMD_LED,
   CMD_USB,
   CMD_SD,
   CMD_RTC,
};

#endif
