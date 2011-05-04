/*****************************************************************************
 * Copyright(c)  2009, GHL Sysytems Berhad.
 *
 * Filename:     atcmd.c
 * Version:      1.0
 * Author:       guowenxue
 * Date:         2012-02-23
 * Description:  发送AT命令并解析返回值
 *
 * Modification History
 *     1.        Version:       1.0
 *               Date:          2012-02-23
 *               Author:        guowenxue
 *               Modification:  creation
 *****************************************************************************/

#include "gprs.h"
#include "comport.h"


#ifndef RECV_LEN
#define RECV_LEN       512
#endif

#undef  ATCMD_DEBUG
#define ATCMD_DEBUG    1

/*****************************************************************************
 * 函数名称: at_match
 * 函数说明: 检查AT命令返回值是否和指定的参数匹配
 * 输入参数: char *pcStr: 
 *           char *pcMatch: 
 * 输出参数: 无
 * 返 回 值: 匹配结果， 0标识匹配成功
 * 调用本函数的函数清单: 
 *           send_atcmd
 *****************************************************************************/
static unsigned char at_match(char *pcStr, char *pcMatch)
{
    char acBuf[256];
    char *pcStart = NULL;
    char *pcTab = NULL;

    pcStart = pcMatch;
    while (0 != pcStart)
    {
        memset(acBuf, 0x00, sizeof(acBuf));

        pcTab = strchr(pcStart, 9); // Find for TAB

        if (0 != pcTab)
        {
            if (pcTab != pcStart)
            {
                strncpy(acBuf, pcStart, pcTab - pcStart);
            }

            pcStart = (0 != *(++pcTab)) ? pcTab : 0;
        }
        else
        {
            strcpy(acBuf, pcStart);
            pcStart = NULL;
        }

        if (0 != acBuf[0] && 0 != strstr(pcStr, acBuf))
        {
            return 0x00;
        }
    }

    return 0x01;
}

/*****************************************************************************
 * 函数名称: send_atcmd
 * 函数说明: 
 * 输入参数: COM_PORT *pstComport: 
 *           char *pcATcmd: 
 *           char *pcExpect: 期望的成功返回值
 *           char *pcError: 期望的错误返回值
 *           unsigned long ulDelay: 发送之前的延时
 *           unsigned long ulGap: 发送之后等待返回值的最长时间 
 *           unsigned long ulTimeout: 串口接收超时
 * 输出参数: 
 *           char *pcReply: 返回值
 *           int iReplyLen: 返回值长度 * 返 回 值: int
 *           
 *****************************************************************************/
int send_atcmd(COM_PORT * pstComport, char *pcATcmd, char *pcExpect,
               char *pcError, unsigned long ulDelay, unsigned long ulGap,
               unsigned long ulTimeout, char *pcReply, int iReplyLen)
{
    unsigned long start = 0;
    int iRet = 0;
    unsigned int uiRecvSize = 0;
    unsigned int uiRecvLen = 0;
    unsigned int uiCmdLen = 0;
    char acRecv[RECV_LEN];
    char *pcRecvPtr = NULL;

      /*=========================================
       *  Pause a while before send AT command
       *=========================================*/
    if (0 != ulDelay)
    {
        start = time_now();
        while (time_elapsed(start) < ulDelay)
        {
            micro_second_sleep(10);
        }
    }

      /*=========================================
       *  Throw rubbish
       *=========================================*/
    memset(&acRecv, 0, sizeof(acRecv));
    start = time_now();
    while (time_elapsed(start) < ulTimeout)
    {
        iRet = comport_recv(pstComport, acRecv, 1, &uiRecvSize, 50);
        if (0 != iRet || 0 == uiRecvSize)
        {
            break;
        }
        micro_second_sleep(10);
    }

      /*=========================================
       *  Send AT command
       *=========================================*/
    uiCmdLen = strlen(pcATcmd);
    iRet = comport_send(pstComport, pcATcmd, uiCmdLen);
    if (0 != iRet)
    {
        iRet = FAILED;
        comport_open(pstComport);   /*Re-open the comport for the FD can not be used */
        goto CleanUp;
    }

      /*=========================================
       * Pause a while before read command response.
       *=========================================*/
    if (0 != ulGap)
    {
        start = time_now();
        while (time_elapsed(start) < ulGap)
        {
            micro_second_sleep(10);
        }
    }

    printf("Start receiving.\n");
      /*=========================================
       *  Start receive and parser data now
       *=========================================*/
    memset(acRecv, 0, sizeof(acRecv));
    pcRecvPtr = acRecv;
    uiRecvLen = 0;

    start = time_now();
    while (time_elapsed(start) < ulTimeout)
    {
        if (uiRecvLen < (RECV_LEN - 1))
        {
            uiRecvSize = 0;
            iRet = comport_recv(pstComport, pcRecvPtr, 1, &uiRecvSize, 50);
            if (0 == iRet && uiRecvSize > 0)
            {
                uiRecvLen += uiRecvSize;
                pcRecvPtr += uiRecvSize;
                acRecv[RECV_LEN - 1] = 0;

                printf("%s\n", acRecv);
                
                      /*==============================================
                       * Match the expect string
                       *=============================================*/
                if (NULL != pcExpect)
                {
                    if (0x00 == at_match(acRecv, pcExpect))
                    {
                        iRet = 0x00;
                        dbg_print("+++++++++++++++++++++++++++++++++Matched \"%s\"\n", pcExpect);
                        goto CleanUp;
                    }
                }

                      /*==============================================
                       * Match the ERROR string
                       *=============================================*/
                if (NULL != pcError)
                {
                    if (0x00 == at_match(acRecv, pcError))
                    {
                        iRet = 0x03;
                        dbg_print("---------------------------------Matched \"%s\"\n", pcError);
                        goto CleanUp;
                    }
                }
            }                   /*Receive data length > 0 */
            else
                micro_second_sleep(1);
        }                       /*Buffer is not overflow */
    }
    printf("Timeout exit.\n");

    if (NULL == pcExpect)
        iRet = 0x00;
    else
        iRet = 0xFF;

  CleanUp:
    if (NULL != pcReply)
    {
        strncpy(pcReply, acRecv, iReplyLen);
    }
    return iRet;
}

/*
 *  Description:   Send "AT" AT command, get "OK" reply means GPRS power up and dataport is OK.
 * Retrun Vaule:   0-> OK       -1-> Failure
 */
/*****************************************************************************
 * 函数名称: atcmd_test
 * 函数说明: 发送at，看是否返回ok
 * 输入参数: COM_PORT *pstComport: 
 * 输出参数: 
 * 返 回 值: int
 * 调用本函数的函数清单: 
 *           
 *****************************************************************************/
int atcmd_test(COM_PORT * pstComport)
{
    int iRet;
    char acReply[256];

    iRet = send_atcmd(pstComport, "AT\r", "OK", "ERROR", 500, 500, 3000, acReply, 256);
#ifdef ATCMD_DEBUG
    printf("\"AT\" Get Reply: \n%s\n", acReply);
#endif
    if (iRet != 0x00)
    {
        return FAILED;          /*Send AT command can not get reply */
    }
    return SUCCESS;             /*Send AT command and get reply ok */
}

/*
 *  Description:   Send "AT" AT command, get "OK" reply means GPRS power up and dataport is OK.
 * Retrun Vaule:   0-> OK       -1-> Failure
 */
int atcmd_ate0(COM_PORT * pstComport)
{
    int iRet;

    iRet = send_atcmd(pstComport, "ATE0\r", "OK", "ERROR", 500, 500, 2000, NULL, 0);
    if (iRet != 0x00)
    {
        return FAILED;          /*Send AT command can not get reply */
    }
    return SUCCESS;             /*Send AT command and get reply ok */
}

/*
 *  Description:   Send "AT+CPIN?" AT command, get "READY" reply means SIM card valid.
 * Retrun Vaule:   0-> OK       -1-> Failure
 */
int check_sim_valid(COM_PORT * pstComport, int iRetries)
{

    int iRet = FAILED;
    int iCount = 0;
    char acReply[256];

    do
    {
        iRet = send_atcmd(pstComport, "AT+CPIN?\r", "READY", "ERROR", 500, 1000, 4000, acReply, 256);
#ifdef ATCMD_DEBUG
        printf("\"AT+CPIN?\" Get Reply: \n%s\n", acReply);
#endif
        if (iRet == 0x00)
        {
            iRet = SUCCESS;     /*Send AT command can not get reply */
            dbg_print("AT+CPIN? inspect valid SIM card.\n");
            break;
        }

        iCount++;
        micro_second_sleep(500);
    }
    while (iCount < iRetries);

    /*Still can not get Reply */
    return iRet;                /*Send AT command and get reply ok */
}

/*
 *  Description:   Send "AT+CREG?" AT command, Check SIM card can regist or not
 *                 Sample:
 *                   at+creg?       // Send Command
 *                   +CREG: 0,1     // Modem reply, 0,1 means Homework  0,5 means Roaming
 *                   OK
 *
 * Retrun Vaule:   0-> OK       -1-> Failure
 *
 */
int check_sim_regist(COM_PORT * pstComport, int iRetries)
{
    int iRet = FAILED;
    char acReply[256];
    char *pcStart = NULL;
    int iCount = 0;
    unsigned char ucRegistMode;

  RETRY:
    iRet = send_atcmd(pstComport, "AT+CREG?\r", "OK", "ERROR", 1000, 1000, 5000, acReply, 256);
    if (iRet != SUCCESS)
    {
        iRet = FAILED;          /*Send AT command can not get reply */
        goto CleanUp;
    }
#ifdef ATCMD_DEBUG
    printf("\"AT+CREG?\" Get Reply: \n%s\n", acReply);
#endif

    pcStart = strstr(acReply, "+CREG: ");
    if (NULL != pcStart)
    {
        pcStart = strchr(pcStart, ',');
        if (NULL != pcStart)
        {
            ucRegistMode = *(pcStart + 1) - '0';
            if (REG_HOMEWORK == ucRegistMode || REG_ROAMING == ucRegistMode)
            {
                iRet = SUCCESS;
                dbg_print("AT+CREG? Check SIM card regist[%d] OK\n", ucRegistMode);
                goto CleanUp;
            }
            else if (REG_DENIED == ucRegistMode || REG_UNKNOW == ucRegistMode)
            {
                iRet = FAILED;
                dbg_print("AT+CREG? Check SIM card regist KO\n") goto CleanUp;
            }
            else if (REG_SEARCHING == ucRegistMode)
            {
                goto RETRY;     /*It's still searching the operator */
            }
        }
    }

    /*Retry sometimes, wait it regist */
    if (iCount < iRetries)
    {
        iCount++;
        micro_second_sleep(500);
        goto RETRY;
    }

  CleanUp:
    printf("Return iRet=%d\n", iRet);
    return iRet;
}

/*
 *  Description:   Send "AT+CSQ" AT command, Check GPRS signal strength
 *                 Sample:
 *                   at+csq          // Send Command
 *                   +CSQ: 19,99     // Modem reply
 *                    OK
 *
 * Retrun Vaule:   0-> OK       -1-> Failure
 *
 */
int check_gprs_signal(COM_PORT * pstComport, int iRetries)
{
    int iRet;
    char acReply[256];
    char *pcStart = NULL;
    char acSignal[16] = { 0 };
    int i = 0;
    int iCount = 0;
    unsigned char ucSignal;

  RETRY:
    iRet = send_atcmd(pstComport, "AT+CSQ\r", "OK", "ERROR", 500, 1000, 5000, acReply, 256);
    if (iRet != 0x00)
    {
        iRet = FAILED;          /*Send AT command can not get reply */
        goto CleanUp;
    }
#ifdef ATCMD_DEBUG
    printf("\"AT+CSQ\" Get Reply: \n%s\n", acReply);
#endif
    pcStart = strstr(acReply, "+CSQ: ");
    if (NULL != pcStart)
    {
        pcStart += strlen("+CSQ: ");

        while (',' != *pcStart && i < 16)
        {
            acSignal[i] = *pcStart;
            i++;
            pcStart++;
        }
        acSignal[i + 1] = '\0';

        if ('\0' != acSignal)
        {
            ucSignal = atoi(acSignal);
            if (ucSignal > 2 && 99 != ucSignal)
            {
                iRet = SUCCESS; /*Get Signal, return OK */
                dbg_print("AT+CSQ Check GPRS signal strength %d\n", ucSignal);
                goto CleanUp;
            }
        }
    }

    /*Can not Get signal, try for some times */
    if (iCount < iRetries)
    {
        iCount++;
        micro_second_sleep(500);
        goto RETRY;
    }

  CleanUp:
    return iRet;
}

/*
 *  Description:   Send "AT+CGCLASS?" AT command, Get GPRS work on 3G or GPRS mode now
 *                 Sample:
 *                   at+cgclass?        // Send Command
 *                   +CGCLASS: "A"      // A->3G B->GSM/GPRS CG->GPRS only CC->GSM Only
 *                   OK 
 *
 * Retrun Vaule:   0-> OK       -1-> Failure
 *
 */
int check_gprs_netype(COM_PORT * pstComport, int iRetries)
{
    int iRet;
    char acReply[256];
    char *pcStart = NULL;
    int iCount = 0;

  RETRY:
    iRet = send_atcmd(pstComport, "AT+CGCLASS?\r", "OK", "ERROR", 500, 500, 3000, acReply, 256);
    if (iRet != 0x00)
    {
        iRet = FAILED;          /*Send AT command can not get reply */
        goto CleanUp;
    }
#ifdef ATCMD_DEBUG
    printf("\"AT+CGCLASS\" Get Reply: \n%s\n", acReply);
#endif
    pcStart = strstr(acReply, "+CGCLASS: ");
    if (NULL != pcStart)
    {
        pcStart += strlen("+CGCLASS: ");
        if (strstr(pcStart, "A"))
        {
            iRet = SUCCESS;
            dbg_print("GPRS working in 3G mode\n");
            goto CleanUp;
        }
        else if (strstr(pcStart, "B"))
        {
            dbg_print("GPRS working in GSM/GPRS mode\n");
            iRet = SUCCESS;
            goto CleanUp;
        }
        else if (strstr(pcStart, "CG"))
        {
            dbg_print("GPRS working in GPRS only mode");
            iRet = SUCCESS;
            goto CleanUp;
        }
        else if (strstr(pcStart, "CC"))
        {
            dbg_print("GPRS working in GSM only mode\n");
            iRet = SUCCESS;
            goto CleanUp;
        }
    }

    /*Can not Get signal, try for some times */
    if (iCount < iRetries)
    {
        iCount++;
        micro_second_sleep(500);
        goto RETRY;
    }

  CleanUp:
    return iRet;

}

/*
 *  Description:   Send "AT+COPS?" AT command, Get the carrier
 *                 Sample:
 *                   at+cops?          // Send Command
 *                   +COPS: 0,0,"China Mobile",0    // Modem reply
 *                   OK
 *
 * Retrun Vaule:   0-> OK       -1-> Failure
 *
 */

int get_carrier(COM_PORT * pstComport)
{
    int iRet;
    char acReply[256];
    char *pcStart = NULL;
    char acCarrier[OPT_LEN];
    int i = 0;
    int iCount = 0;

  RETRY:
    memset(acCarrier, 0, OPT_LEN);
    iRet = send_atcmd(pstComport, "AT+COPS?\r", "OK", "ERROR", 200, 500, 5000, acReply, 256);
    if (iRet != 0x00)
    {
        iRet = FAILED;          /*Send AT command can not get reply */
        goto CleanUp;
    }
#ifdef ATCMD_DEBUG
    printf("\"AT+COPS?\" Get Reply: \n%s\n", acReply);
#endif

    pcStart = strchr(acReply, '\"');
    if (NULL != pcStart)
    {
        pcStart++;
        while ('\"' != *pcStart && i < OPT_LEN)
        {
            acCarrier[i] = *pcStart;
            pcStart++;
            i++;
        }
        acCarrier[i + 1] = '\0';
    }

    if ('\0' != acCarrier)
    {
        dbg_print("Carrier: \"%s\"\n", acCarrier);
        iRet = SUCCESS;         /*Send AT command and get reply ok */
        goto CleanUp;
    }

    /*Can not Get carrrier, try for another 3 times */
    if (iCount < 8)
    {
        iCount++;
        micro_second_sleep(500);
        goto RETRY;
    }

  CleanUp:
    return iRet;
}

/*
 * Send AT command pcCmd to shutdown the GPRS module
 */
int atcmd_poweroff(COM_PORT * pstComport, char *pcCmd)
{
    int iRet;

    iRet = send_atcmd(pstComport, pcCmd, NULL, NULL, 500, 0, 1000, NULL, 0);
    if (iRet != 0x00)
    {
        iRet = FAILED;          /*Send AT command can not get reply */
        goto CleanUp;
    }

  CleanUp:
    return iRet;
}

/*
 *  Description:   Send "AT+CGDCONT" AT command, Set APN for the GPRS.
 * Retrun Vaule:   0-> OK       -1-> Failure
 */
/*****************************************************************************
 * 函数名称: atcmd_set_apn
 * 函数说明: 通过at设置apn
 * 输入参数: GPRS *pstGprs: Datachn
 *           APN *apn: 指定的apn
 * 输出参数: 无
 * 返 回 值: 操作结果
 * 调用本函数的函数清单: 
 *           set_apn
 *****************************************************************************/
int atcmd_set_apn(COM_PORT * pstComport, APN * apn)
{
    int iRet = SUCCESS;
    char acCmd[128] = { 0 };
    char acReply[256];

    snprintf(acCmd, sizeof(acCmd), "AT+CGDCONT=1,\"IP\",\"%s\"\r", apn->acApn);
#ifdef ATCMD_DEBUG
    printf("\"AT+CGDCONT=1,\"IP\",\"%s\" Get Reply: \n%s\n", apn->acApn, acReply);
#endif
    iRet = send_atcmd(pstComport, acCmd, "OK", "ERROR", 500, 0, 2500, acReply, 256);
    if (iRet != 0x00)
    {
        iRet = FAILED;          /*Send AT command can not get reply */
        goto CleanUp;
    }

  CleanUp:
    return iRet;
}
