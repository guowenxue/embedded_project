/*
 * =====================================================================================
 *
 *       Filename:  gprs.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/22/2010 10:15:11 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Guo Wenxue (kkernel), guowenxue@gmail.com
 *
 * =====================================================================================
 */

#include "gprs.h"

char recv_buf[SIZE_512];

int ctrlport_open(CTRL_PORT *ctrlport)
{
    if( (ctrlport->fd=open(ctrlport->dev_name, O_RDWR, 0666) ) < 0 )
    {
         logs(LOG_CRIT, "Open GPRS control port \"%s\" failure.\n", ctrlport->dev_name);
         return -1;
    }
    return 0;
}

int ctrlport_close(CTRL_PORT *ctrlport)
{
    if( close(ctrlport->fd) < 0 )
    {
         logs(LOG_CRIT, "Close GPRS control port \"%s\" failure.\n", ctrlport->dev_name);
         return -1;
    }
    return 0;
}

int open_gprs(GPRS_ATTR *gprs)
{
     CTRL_PORT   *ctrlport = &(gprs->ctrl_port);
     COM_PORT    *comport = &(gprs->data_port);

     logs(LOG_DEBG, "Open control port: %s comport: %s\n", ctrlport->dev_name, comport->dev_name);

     /* Open /dev/gprs */
     if( (ctrlport_open(ctrlport)) < 0 )
            return -1;

     init_comport(comport); 
     if( 0 != comport_open(comport) )
     {
        logs(LOG_CRIT, "Open comport \"%s\" failure.\n", comport->dev_name);
        ctrlport_close(ctrlport); /*   */
        return -1;
     }

     return 0;
}

int term_gprs(GPRS_ATTR *gprs)
{
    CTRL_PORT   *ctrlport = &(gprs->ctrl_port);
    COM_PORT    *comport = &(gprs->data_port);

    ctrlport_close(ctrlport);
    close(comport->fd_com);

    return 0x00;
}

int ioctl_check_sim_door(GPRS_ATTR *gprs, int sim)
{
    if((SIM1!=sim) && (SIM2!=sim))
    {
          logs(LOG_ERRO, "SIM%d not support, only support SIM1, SIM2.\n", sim);
          return -1;
    }
#ifdef ARCH_ARM9
    int         iRet = -1;
    iRet=ioctl(gprs->ctrl_port.fd, GPRS_CHKSIM, 1);
    if(iRet < 0)
            return -1;
    else if(0==iRet)
       gprs->door[sim] = SIMDOOR_CLOSED;
    else
       gprs->door[sim] = SIMDOOR_OPENED;
#else
    gprs->door[sim] = ioctl(gprs->ctrl_port.fd, GPRS_CHKSIM, 0);
#endif
    return gprs->door[sim];
}

/*Return Value: 0x00->RING_NOEN, 0x01->RING_SMS, 0x02->RING_CALL*/
int ioctl_check_gprs_ring(GPRS_ATTR *gprs)
{
    int        iRet = -1;
    iRet = ioctl(gprs->ctrl_port.fd, GPRS_CHKRING, 0);
    return iRet;
}

int gprs_power_up(GPRS_ATTR *gprs)
{
     int                retries = 5;
     unsigned char      ucRet = 0x01;

     if(POWER_ON == atcmd_check_gprs_power(gprs))
     {
        logs(LOG_INFO, "GPRS Module already power up now.\n");
        return 0;
     }

#ifdef ARCH_L350
     if( ioctl(gprs->ctrl_port.fd, GPRS_POWERON, SIM1) < 0)
#else
     if( ioctl(gprs->ctrl_port.fd, GPRS_POWERON, 0) < 0)
#endif
     {
        logs(LOG_CRIT, "ioctl power up GPRS module failure.\n");
        return -1;
     }

RETRY:
     ucRet = atcmd_check_gprs_power(gprs);
     if( (POWER_ON!=ucRet) && retries>0)
     {
        sleep(2);
        retries--;
        goto RETRY;
     }

     if(POWER_ON == ucRet)
         return 0;
     else
         return -1;
}

int gprs_power_down(GPRS_ATTR *gprs)
{
    unsigned char          ucRet;
    COM_PORT               *comport = &(gprs->data_port);

    ucRet = send_atcmd(comport, "AT+CFUN=0\r", "OK", "ERROR", 1000, 500, 2000, recv_buf);
    if(0x00 == ucRet)
    {
       sleep(5); /* Must sleep for 3 seconds to make sure module is power off  */
       gprs->power = POWER_OFF;
       return 0;
    }
    goto CHECK;

HARDOFF:
    logs(LOG_DEBG, "Try to use ioctl to shutdown the GPRS module.\n");
    if( ioctl(gprs->ctrl_port.fd, GPRS_POWERDOWN, 0) < 0 )
       return -1;
    sleep(3);

CHECK:
    msleep(3000);
    ucRet = send_atcmd(comport, "AT\r", "OK", "ERROR", 1000, 500, 300, recv_buf);
    if(0x00 == ucRet)
    {
       goto HARDOFF; 
    }

    logs(LOG_DEBG, "GPRS already Power Off.\n");
    gprs->power = POWER_OFF;
    return 0;
}

unsigned char atcmd_check_gprs_power(GPRS_ATTR *gprs)
{
   unsigned  ucRet; 
   COM_PORT    *comport = &(gprs->data_port);

   ucRet = send_atcmd(comport, "ATE0\r", "OK", "ERROR", 500, 100, 300, recv_buf);
   if(0x00 != ucRet)
   {
       gprs->power = POWER_OFF;
       logs(LOG_DEBG, "ATE0 Check GPRS Power Status:    [OFF]\n");
       goto RET;
   }

   logs(LOG_DEBG, "ATE0 Check GPRS Power Status:    [ON]\n");
   gprs->power = POWER_ON;

RET:
   return gprs->power;
}

unsigned char atcmd_check_sim_insert(GPRS_ATTR *gprs)
{
   COM_PORT    *comport = &(gprs->data_port);
   unsigned    ucRet; 

   ucRet = send_atcmd(comport, "AT+CPIN?\r", "OK", "ERROR", 500, 100, 300, recv_buf);
   if(0x00 != ucRet)
   {
       gprs->work_sim->insert = 0x00;
       logs(LOG_ERRO, "AT+CPIN? Check SIM Insert:    [FAILURE]\n");
       goto RET;
   }

   gprs->work_sim->insert = 0x01;
   logs(LOG_DEBG, "AT+CPIN? Check SIM Insert:    [OK]\n");
RET:
   return ucRet;
}

unsigned char atcmd_check_gprs_signal(GPRS_ATTR *gprs)
{
   char              *pcStart = NULL;
   char              *pcEnd = NULL;
   unsigned  char    ucRet;
   int               signal;
   char              acSignal[4]={0};
   COM_PORT          *comport = &(gprs->data_port);

   ucRet = send_atcmd(comport, "AT+CSQ\r", "OK", "ERROR", 500, 100, 300, recv_buf);
   if(0x00 == ucRet)
   {
        pcStart = strstr (recv_buf, "+CSQ: ");   
        if (NULL != pcStart)
        {
             pcStart += 6; 
             pcEnd = strchr (pcStart, ','); 
             if (NULL!=pcEnd && 0<(pcEnd-pcStart) && sizeof(acSignal)>(pcEnd-pcStart))
             {
                  strncpy (acSignal, pcStart, pcEnd-pcStart);

                  signal=atoi(acSignal);
                  if (0 < signal && 31 >= signal)
                  {
                      logs(LOG_DEBG, "Get GPRS Signal: %d\n", signal);
                      gprs->signal = signal; 
                      return 0x00;
                  }
             }
        }
   }

   logs(LOG_ERRO, "Get GPRS Signal failure: \n", recv_buf);
   return 0x01;
}

unsigned char atcmd_check_gprs_register(GPRS_ATTR *gprs)
{
   unsigned  char    ucRet;
   char              acRes[SIZE_128];
   char              acRegist[4]={0};
   int               iRegist;
   int               iRetries = 6;
   char              *pcStart = NULL;
   char              *pcEnd = NULL;
   COM_PORT          *comport = &(gprs->data_port);

RETRY:
   strcpy (acRes, "+CREG: 0,1\r\n\r\nOK    +CREG: 1,1\r\n\r\nOK    +CREG: 0,5\r\n\r\nOK    +CREG: 1,5\r\n\r\nOK");
   ucRet = send_atcmd(comport, "AT+CREG?\r", "OK", "ERROR", 100, 0, 1000, recv_buf);
   if(0x00 == ucRet) /* Get Reply */
   {
      pcStart = strstr (recv_buf, "+CREG: "); /*pcStart: "+CREG: 1,1***" */
      if(NULL != pcStart)
      {
           pcStart += 7; /* pcStart: "1,1****" */  
           pcStart = strchr (pcStart, ','); /* pcStart: ",1*****" */
           if (NULL != pcStart)
           {
               pcStart += 1; 
               pcEnd = strchr (pcStart, '\r');

               if (NULL!=pcEnd && 0<(pcEnd-pcStart) &&  sizeof(acRegist)>(pcEnd-pcStart) )
               {
                        strncpy (acRegist, pcStart, pcEnd - pcStart);    
                        iRegist = atoi(acRegist);

                        if( REG_HOMEWORK == iRegist )
                        {
                           logs(LOG_DEBG, "SIM card Regist Home Work.\n");
                           gprs->work_sim->regist = REG_HOMEWORK;
                           return 0x00;
                        }
                        else if(REG_ROAMING == iRegist)
                        {
                           logs(LOG_DEBG, "SIM card Regist Roaming.\n");
                           gprs->work_sim->regist = REG_ROAMING;
                           return 0x00;
                        }
               }

           } /* NULL!=pcStart */

      } /* NULL!=pcStart */

   } /* 0x00==ucRet */

   if(iRetries > 0)
   {
       sleep(3); 
       iRetries--;
       goto RETRY;
   }

   logs(LOG_DEBG, "Check SIM card Regist:    [FAILURE].\n");
   gprs->work_sim->regist = REG_UNREGIST;
   return 0x01;
}


unsigned char  atcmd_check_gprs_carrier(GPRS_ATTR *gprs)
{
   unsigned  char    ucRet; 
   int               i = 0;
   int               iRetries = 4;
   char              *pcStart = NULL; 
   COM_PORT          *comport = &(gprs->data_port);

RETRY:
   ucRet = send_atcmd(comport, "AT+COPS?\r", "OK", "ERROR", 100, 100, 4000, recv_buf);
   if(0x00 == ucRet)
   {
       pcStart = strchr(recv_buf, '\"');
       if( NULL != pcStart )
       {
           while( '\"' != *(++pcStart) && i<APN_LEN) 
           {
               gprs->work_sim->carrier[i] = *pcStart; 
               i++;
           }
       }
       gprs->work_sim->carrier[i++] = '\0'; 
       logs(LOG_DEBG, "Get Network Operation: \"%s\"\n", gprs->work_sim->carrier);
       if('\0' != gprs->work_sim->carrier[0])
       {
           return 0x00;
       }
   }

   if(iRetries > 0)
   {
      iRetries --; 
      goto RETRY;
   }

   gprs->work_sim->carrier[0]='\0';
   return 0x01;
}

unsigned char atcmd_set_apn(GPRS_ATTR  *gprs)
{
   char             atcmd[64]={0};
   unsigned char    ucRet;
   COM_PORT         *comport = &(gprs->data_port);

   sprintf (atcmd, "AT+CGDCONT=1,\"IP\",\"%s\"\r", gprs->work_sim->APN);
   if(0x00!= (ucRet=send_atcmd(comport, atcmd, "OK", "ERROR", 100, 0, 2000, recv_buf)) )
   {
         logs(LOG_CRIT, "AT command \"AT+CGDCONT\" set APN failure.\n");
   }
   else
   {
         logs(LOG_DEBG, "AT command \"AT+CGDCONT\" set APN.         [OK]\n");
   }
   return ucRet;
}

unsigned char check_gprs_status(GPRS_ATTR *gprs)
{
  //   unsigned char ucRet;
     int       retries = 5;  /* Try to get GPRS signal times */

     if( 0x00 != atcmd_check_sim_insert(gprs) )
         return 0x01;

RETRY:
     if(0x00 != g_ucStop)
         return 0x02;
     if( 0x00 != atcmd_check_gprs_signal(gprs) )
     {
         if(retries > 0)
         {
             retries --;
             sleep(3);
             goto RETRY;
         }
     }

     if( 0x00 != atcmd_check_gprs_register(gprs) )
     {
         goto RETRY;
     }

     if( 0x00 != atcmd_check_gprs_carrier(gprs) )
     {
         goto RETRY; 
     }

     atcmd_check_gprs_signal(gprs); /* Check GPRS signal again to update the signal value. */

     logs(LOG_DEBG, "Check GPRS SIM card stauts:    [OK]\n");
     return 0x00;
}

unsigned char at_match (char *p_pcStr, char *p_pcMatch)
{
        char                    acBuf [256],
                                        *pcStart                = NULL,
                                        *pcTab                  = NULL;

        pcStart = p_pcMatch;

        while (0 != pcStart)
        {
                memset (acBuf, 0x00, sizeof (acBuf));

                pcTab = strchr (pcStart, 9);    // Find for TAB

                if (0 != pcTab)
                {
                        if (pcTab != pcStart)
                        {
                                strncpy (acBuf, pcStart, pcTab - pcStart);
                        }

                        pcStart = (0 != *(++pcTab)) ? pcTab : 0;
                }
                else
                {
                        strcpy (acBuf, pcStart);
                        pcStart = NULL;
                }

                if (0 != acBuf [0] && 0 != strstr (p_pcStr, acBuf))
                {
                        return 0x00;
                }
        }

        return 0x01;
}

 
/*=========================================================================================================
 * Parameter Description:  
 *     COM_PORT *st_comport:  The GPRS module data port(/dev/ttyS2);  
 *     char          *atCmd:  The AT command which will be sent to GPRS module
 *     char         *expect:  The EXPECT reply string by GPRS module for the AT command, such as "OK"
 *     char          *error:  The ERROR  reply string by GPRS module for the AT command, such as "ERROR"
 *     unsigned long  delay:  Before send the AT command, we pause a while to wait last AT reuturned. 
 *     unsigned long    gap:  After send the AT command, we pause a while for modem to reply for it.
 *   unsigned long  timeout:  Read from data port timeout value
 *     char        *content:  The AT command reply contont
 *
 * Return Value:
 *     unsigned char ucRet:   0x00->command send OK and "expect" string mached.
 *     char       *content:   The AT command reply string by modem.
 *
 *=========================================================================================================*/

unsigned char send_atcmd(COM_PORT *st_comport, char *atCmd, char *expect, char *error,
               unsigned long delay, unsigned long gap, unsigned long timeout, char *content)
{
     unsigned char        ucRet = 0x00;
     unsigned long        ulCurTime, ulStartTime;

     int                  iCmdLen = 0,
                          iRecvLen = 0,
                          iRecvSize = 0,
                          i,
                          iRet = 0;

     char                 acRecv[SIZE_1024];
     char                 *pcRecvPtr = NULL;

     if(st_comport->is_connted != 0x01) /* Comport not opened */
     {
        logs(LOG_ERRO, "Comport not opened.\n");
        return  0x01;
     }

     /*=========================================
      *=  Pause a while before send AT command =
      *=========================================*/
     if(0 != delay)
     {
         GET_TICKCOUNT(ulCurTime);
         ulStartTime = ulCurTime;
         while ( (ulCurTime-ulStartTime)<delay && ulStartTime<=ulCurTime )
         {
             NSLEEP(1);
             GET_TICKCOUNT(ulCurTime);
         }
     }

     /*====================
      *=  Throw Rubbish   =
      *====================*/
     GET_TICKCOUNT(ulCurTime);
     ulStartTime = ulCurTime;

     memset(&acRecv, 0, sizeof(acRecv));
     while ( (ulCurTime-ulStartTime)<delay && ulStartTime<=ulCurTime )
     {
         ucRet = comport_recv(st_comport, (unsigned char*)acRecv, 1, &iRet, 50);
         if(0x00!=ucRet || 0==iRet)
         {
            break;
         }

         NSLEEP(1);
         GET_TICKCOUNT(ulCurTime);
     }

     /*====================
      *=  Send AT command =
      *====================*/

     iCmdLen = strlen(atCmd);
     ucRet = comport_send (st_comport, (unsigned char *)atCmd, iCmdLen);
     if (0 != ucRet)
     {
         ucRet = 0x02;
         goto  CleanUp;
     }


     /*===================================================
      *=  Pause a while before read command response. 
      *===================================================*/
     if(0 != gap)
     {
        GET_TICKCOUNT (ulCurTime);
        ulStartTime = ulCurTime;
        while ( (ulCurTime-ulStartTime)<gap && ulStartTime<=ulCurTime )
        {
             NSLEEP(1);
             GET_TICKCOUNT(ulCurTime);
        }
     }

      memset (acRecv, 0, sizeof (acRecv));
      pcRecvPtr = acRecv;
      iRecvLen = 0;
      iRecvSize = sizeof (acRecv);

      GET_TICKCOUNT (ulCurTime);
      ulStartTime = ulCurTime;

      while ( (ulCurTime-ulStartTime)<timeout && ulStartTime<=ulCurTime )
      {
         if ( iRecvLen < (iRecvSize-1) )
         {
              ucRet = comport_recv (st_comport, (unsigned char *) pcRecvPtr, 1, &iRet, 50);

              if (0x00==ucRet && 0<iRet)
              {
                   iRecvLen += iRet;
                   pcRecvPtr += iRet;
                   acRecv [iRecvSize-1] = 0;

                   /*========================================
                    * Match the received with expect String =
                    *========================================*/
                   if(NULL != expect)
                   {
                     if (0x00 == at_match(acRecv, expect))
                     {
                        ucRet = 0x00;
                        goto CleanUp;
                     }
                   }

                   /*========================================
                    * Match the received with error String  =
                    *========================================*/
                  if(NULL != error)
                  {
                     if (0x00 == at_match(acRecv, error))
                     {

                        ucRet = 0x03;
                        goto CleanUp;
                     }
                  }

              } /*End of (0x00==ucRet && 0<iRet)  */

         } /* End of (iRecvLen < (iRecvSize-1))  */
         NSLEEP(1);
         GET_TICKCOUNT (ulCurTime);
      } /* End of ( (ulCurTime-ulStartTime)<timeout )*/

      if(NULL==expect)
         ucRet = 0x00;
      else
         ucRet = 0xFF;

CleanUp:
      if( NULL != content)
      {
            memset(content, 0, SIZE_1024);
            strncpy(content, acRecv, SIZE_1024);
      }

      if(g_ucDebugLev > LOG_DEBG)
      {
          char   log[SIZE_512] = {0};
          snprintf(log, SIZE_512, "Send AT command: \"%s\" get reply \"%s\"", atCmd, acRecv);
          for (i=0; i<SIZE_512; i++)
          {
            if('\r'==log[i] || '\n'==log[i] || '\t'==log[i]) 
            {
                  log[i]=' '; 
            }
            else if (0 == log[i])
              break;
          }
          logs(LOG_INFO, "%s\n", log );
      }

      return ucRet;
}
