/*********************************************************************************
 *  Copyright(c)  2011, GHL Systems 
 *  All ringhts reserved.
 *
 *     Filename:  dev_sam.c
 *  Description:  AT91SAM9XXX IS07816 Smart Card driver
 *
 ********************************************************************************/
#include "include/plat_driver.h"

#define DRV_DESC                  "AT91SAM9XXX SAM card driver"

/*Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_SAM_NAME

#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*dynamic major by default */
#endif

static struct cdev *dev_cdev = NULL;
static struct class *dev_class = NULL;
	
static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;
static uint m_ucVccType = VccType3V3;

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);
module_param(m_ucVccType, uint, S_IRUGO);

#define DIRECT_MODE 0
#define INVERSE_MODE    1
#define CARD_BUF_SIZE 1024
#define USART_SEND 0
#define USART_RCV  1
#define MAX_ATR_SIZE            55
#define ATR_FILED_NAME_SIZE 8
#define PROTOCOL_TYPE_T0    0x00    /* Protocol type T=0 */
#define PROTOCOL_TYPE_T1    0x01    /* Protocol type T=1 */
#define PROTOCOL_TYPE_T2    0x02    /* Protocol type T=2 */
#define PROTOCOL_TYPE_T3    0x03    /* Protocol type T=3 */
#define PROTOCOL_TYPE_T14   0xFE    /* Protocol type T=14 */
#define PROTOCOL_TYPE_T15   0xFF    /* Protocol type T=15 */

volatile unsigned int *PMC_PCER;
volatile unsigned int *US_CR;
volatile unsigned int *US_MR;
volatile unsigned int *US_CSR;
volatile unsigned int *US_RHR;
volatile unsigned int *US_THR;
volatile unsigned int *US_IDR;
volatile unsigned int *US_TTGR;
volatile unsigned int *US_BRGR;
volatile unsigned int *US_FIDIR;
volatile unsigned int *US_NER;
volatile unsigned int *US_RTOR;

enum __ATRDecInfoIndex {
    inverse_logic = 0,
    ATRProtocolType,
    num_historical_bytes,
    TA_exist,
    TB_exist,
    TC_exist,
    TD_exist,
    ATR_FI_decimal,
    ATR_DI_decimal,
    Extra_Guard_Time,
    max_ATRDecInfoIndex
} ATRDecInfoIndex;

typedef struct __ATRField {
    unsigned char FieldBuf[MAX_ATR_SIZE];
    char FieldName[MAX_ATR_SIZE][ATR_FILED_NAME_SIZE];
    int ATRDecResult[max_ATRDecInfoIndex]; /* the buffer to store card property related info which decode from ATR */
    unsigned char len;
} ATRField;

static ATRField ATRBuf; /* the buffer to receive ATR response from card */

typedef struct __cardAnswer {
    unsigned char buf[CARD_BUF_SIZE];
    int len;
} cardAnswer;

static cardAnswer APDUAnswerBuf; /* the buffer to receive data from card when transmit APDU */

static unsigned char protocolType = PROTOCOL_TYPE_T0;
static int card_logic_mode = DIRECT_MODE;
//static int card_logic_mode = INVERSE_MODE;
static unsigned char StateUsartGlobal = USART_RCV;
static int dev_open_count = 0; // prevent concurrent access to the same device
static DEFINE_SPINLOCK(spin);

#define dbg_print(format,args...) if(DISABLE!=debug) \
                                  {printk("[kernel] ");printk(format, ##args);}

/*This Clock Management part code add by guowenxue, to get the Master clock for SAM card*/
#define ARCH_ID_AT91SAM9260 0x019803a0
#define ARCH_ID_AT91SAM9G20 0x019905a0

static unsigned long main_clk_rate_hz;
static unsigned long cpu_clk_rate_hz;
//static unsigned long mck_rate_hz = 99328000;
static unsigned long mck_rate_hz = BOARD_MCK;
static unsigned long plla_rate_hz;

static u32 at91_pll_rate(u32 freq, u32 reg)
{ 
    unsigned mul, div; 

    div = reg & 0xff; 
    mul = (reg >> 16) & 0x7ff; 
    if (div && mul) 
    { 
        freq /= div; 
        freq *= mul + 1; 
    } 
    else 
        freq = 0; 
    
    return freq;
}

static unsigned long at91_css_to_rate(unsigned long css)
{
    switch (css) 
    { 
            case AT91_PMC_CSS_SLOW: 
                    return AT91_SLOW_CLOCK; 
            case AT91_PMC_CSS_MAIN: 
                    return main_clk_rate_hz; 
            case AT91_PMC_CSS_PLLA: 
                    return plla_rate_hz; 
#if 0
            case AT91_PMC_MCKR_CSS_PLLB: 
                    return pllb_rate_hz; 
#endif
    } 
    return 0;
}


int at91_get_clock(unsigned long main_clock)
{
    unsigned regs, freq;
    main_clk_rate_hz = main_clock;

    regs = at91_sys_read(AT91_CKGR_PLLAR);
    plla_rate_hz = at91_pll_rate(main_clock, regs);

    regs = at91_sys_read(AT91_PMC_MCKR);
    mck_rate_hz = at91_css_to_rate(regs & AT91_PMC_CSS);
    freq = mck_rate_hz;
    freq /= (1 << ((regs & AT91_PMC_PRES) >> 2)); /* prescale */

    if(ARCH_ID_AT91SAM9G20 == (at91_sys_read(AT91_DBGU_CIDR) & ~AT91_CIDR_VERSION))
    {
        /* mdiv ; (x >> 7) = ((x >> 8) * 2) */
        mck_rate_hz = (regs & AT91_PMC_MDIV) ?
            freq / ((regs & AT91_PMC_MDIV) >> 7) : freq;
        if (regs & AT91_PMC_MDIV)
            freq /= 2;          /* processor clock division */
    }
    else if(ARCH_ID_AT91SAM9G20 == (at91_sys_read(AT91_DBGU_CIDR) & ~AT91_CIDR_VERSION))
    {
        mck_rate_hz = freq / (1 << ((regs & AT91_PMC_MDIV) >> 8)); 
    }

    cpu_clk_rate_hz = freq;

#if 0
    printk("Clocks: CPU %lu MHz, master %u MHz, main %u.%03u MHz\n",
            cpu_clk_rate_hz / 1000000, (unsigned) mck_rate_hz / 1000000,
            (unsigned) main_clk_rate_hz / 1000000, ((unsigned) main_clk_rate_hz % 1000000) / 1000);
#endif
    return 0;
}
/*Add by guowenxue 2011.08.23*/
								  
static inline unsigned char swapbits(unsigned char i)
{
    return ( 0xff ^ i );
}

static unsigned int if_dev_send_char( unsigned char CharToSend )
{
    unsigned int status;
    unsigned char senddata;
	
    if( StateUsartGlobal == USART_RCV )
    {
        /* AT91C_US_RSTSTA: Reset Status Bit PARE,FRAME,OVER,MANERR,RXBRK in US_CSR
         * AT91C_US_RSTIT: Reset ITERATION in US_CSR only for ISO7816 mode
         * AT91C_US_RSTNACK: Reset NACK in US_CSR
         */
        *US_CR = AT91C_US_RSTSTA | AT91C_US_RSTIT | AT91C_US_RSTNACK;
        StateUsartGlobal = USART_SEND;
        /*Enable TX and Disable RX*/
        *US_CR = AT91C_US_TXEN | AT91C_US_RXDIS;
    }

    if (card_logic_mode == INVERSE_MODE) 
	{
		senddata =  swapbits(CharToSend);
		*US_MR |= AT91C_US_MSBF;
    }
    else
	{
	    senddata =  (CharToSend);
	    *US_MR &= (~AT91C_US_MSBF);
    }

    /*Wait for TX ready*/
    while(((*US_CSR) & AT91C_US_TXRDY) == 0)
           {;}

    /*Send the data*/
    *US_THR = senddata;

    status = ((*US_CSR)&(AT91C_US_OVRE|AT91C_US_FRAME|
						AT91C_US_PARE|AT91C_US_NACK|
						AT91C_US_ITERATION));

    if (status != 0 ) 
	{
		dbg_print("send error: US_CSR=0x%x\n", *US_CSR);
		dbg_print("number of errors: 0x%x\n", *US_NER);
        *US_CR = AT91C_US_RSTSTA | AT91C_US_RSTIT | AT91C_US_RSTNACK;
		return status;
    }
    return 0;
}

static unsigned int if_dev_get_char( unsigned char *pCharToReceive )
{
    unsigned int status;
    unsigned char revdata;
    unsigned long timeout;
 
    //*US_CR = AT91C_US_STTTO; //Flush the last Start Time-out command, add by guowenxue

    timeout = jiffies+HZ; // set timeout as 1s

    if( StateUsartGlobal == USART_SEND )
    {
        /*If the UART is in Send mode and there is charactor ready to send, then wait send over*/
        while(((*US_CSR) & AT91C_US_TXEMPTY) == 0)
		{;}
		*US_CR = AT91C_US_RSTSTA | AT91C_US_RSTIT | AT91C_US_RSTNACK;
		StateUsartGlobal = USART_RCV;
        /*Enable RX and Disable TX*/
		*US_CR = AT91C_US_RXEN | AT91C_US_TXDIS;
        timeout = jiffies+HZ/2; // set timeout as 1s
    }

    if (card_logic_mode == INVERSE_MODE) 
	{
	    *US_MR |= AT91C_US_MSBF; 
    }
    else
	{
	    *US_MR &= (~AT91C_US_MSBF);
    }

    /* Flush the last Start Time-out command, add by guowenxue */
    *US_CR = AT91C_US_STTTO; 

    while( (((*US_CSR) & AT91C_US_RXRDY) == 0) )
    {
		if((*US_CSR) & AT91C_US_TIMEOUT)
		{
		    *US_CR = AT91C_US_STTTO;
		    printk("receive timeout\n");
		    return TIMEROUT_ERR;
		}

		if (time_before_eq(timeout, jiffies)) //time out
		{
			printk("TIMEOUT\n");
			return TIMEROUT_ERR;
		}
	}

	revdata = ((*US_RHR) & 0xff);
 
    if (card_logic_mode == INVERSE_MODE) 
	{
		*pCharToReceive =  swapbits(revdata);
    }
    else
	{
		*pCharToReceive =  (revdata);
    }

	status = ((*US_CSR)&(AT91C_US_OVRE|AT91C_US_FRAME|
			AT91C_US_PARE|AT91C_US_TIMEOUT|AT91C_US_NACK));

    if (status != 0 ) 
	{
        printk("receive error: US_CSR=0x%x\n", *US_CSR);
        printk("number of errors: 0x%x\n", *US_NER);
        *US_CR = AT91C_US_RSTSTA | AT91C_US_RSTNACK | AT91C_US_STTTO;
        return status;
    }
    return 0;
}

static void card_set_power(int cmd)
{
    if (cmd) {
        dbg_print("Set SAM card power: 0x%02x\n", cmd);
        if (VccType3V3==m_ucVccType) 
		{
			at91_set_gpio_value( SAM_M0, 0 );
			at91_set_gpio_value( SAM_M1, 1 );
#ifdef PLAT_L2
			at91_set_gpio_value( SAM_M2, 1 );
#endif
        }
        else if (VccType5V==m_ucVccType) 
		{
			at91_set_gpio_value( SAM_M0, 1 );
			at91_set_gpio_value( SAM_M1, 1 );
#ifdef PLAT_L2
			at91_set_gpio_value( SAM_M2, 1 );
#endif
        }
#ifdef PLAT_L2
        else if (VccType3V==m_ucVccType) 
		{
			at91_set_gpio_value( SAM_M0, 1 );
			at91_set_gpio_value( SAM_M1, 0 );
			at91_set_gpio_value( SAM_M2, 0 );
        }
        else if (VccType1V8==m_ucVccType) 
		{
			at91_set_gpio_value( SAM_M0, 1 );
			at91_set_gpio_value( SAM_M1, 0 );
			at91_set_gpio_value( SAM_M2, 1 );
        }
#endif
    }
    else /*0V*/
	{
		at91_set_gpio_value( SAM_M0, 0 );
		at91_set_gpio_value( SAM_M1, 0 );
#ifdef PLAT_L2
		at91_set_gpio_value( SAM_M2, 0 );
#endif
    }
}

static void card_set_reset(int cmd)
{
    if (cmd) 
	{
		at91_set_gpio_value( SAM_RST, 1);
    }
    else 
	{
		at91_set_gpio_value( SAM_RST, 0);
    }
}

static void card_set_clock(int cmd)
{
    if (cmd) 
	{
        *US_BRGR = mck_rate_hz / (372*9600);
    }
    else 
	{
        *US_BRGR = 0;
    }
}

static void card_activate(void)
{
    card_set_reset(0);
    card_set_power(m_ucVccType);
    *US_CR = AT91C_US_RXEN | AT91C_US_TXDIS;
    card_set_clock(1);
}

static void card_deactivate(void)
{
    card_set_reset(1);
    card_set_clock(0);
    /*Reset RXD/TXD and Disable RXD/TXD*/
    *US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RXDIS | AT91C_US_TXDIS;
    //*US_FIDIR = 372;          
    card_set_power(0);
}

static void card_cold_reset(void)
{
    msleep(10);
    *US_CR = AT91C_US_RSTSTA | AT91C_US_RSTIT | AT91C_US_RSTNACK;
    card_set_reset(1);
}

static void card_hot_reset(void)
{
    card_set_reset(0);
    msleep(10);
    *US_CR = AT91C_US_RSTSTA | AT91C_US_RSTIT | AT91C_US_RSTNACK;
    card_set_reset(1);
}

static int card_get_ATR(ATRField* pAtr)
{
	unsigned int i;
	unsigned int j;
	unsigned int y;
	unsigned char checkSum;
	unsigned int offset;

	pAtr->len = 0;

    // Read ATR TS
    if (if_dev_get_char(&pAtr->FieldBuf[0])) {
        printk("failed to get ATR TS\n");
        return -1;
    }
    checkSum = pAtr->FieldBuf[0] ^ 0x0;
    // Read ATR T0
    if (if_dev_get_char(&pAtr->FieldBuf[1])) {
        printk("failed to get ATR T0\n");
        return -1;
    }
    checkSum ^= pAtr->FieldBuf[1];
    y = pAtr->FieldBuf[1] & 0xF0;
    i = 2;

    // Read ATR Ti
    offset = 1;
    while (y) {

		if (y & 0x10) {  // TA[i]
			if_dev_get_char(&pAtr->FieldBuf[i]);
			if (offset==2) {
				pAtr->ATRDecResult[ATRProtocolType] |= pAtr->FieldBuf[i]&0x0F;
			}
			checkSum ^= pAtr->FieldBuf[i++];
		}
		if (y & 0x20) {  // TB[i]
			if_dev_get_char(&pAtr->FieldBuf[i]);
			checkSum ^= pAtr->FieldBuf[i++];
		}
		if (y & 0x40) {  // TC[i]
			if_dev_get_char(&pAtr->FieldBuf[i]);
			checkSum ^= pAtr->FieldBuf[i++];
		}
		if (y & 0x80) {  // TD[i]
			if_dev_get_char(&pAtr->FieldBuf[i]);
			pAtr->ATRDecResult[ATRProtocolType] |= pAtr->FieldBuf[i]&0x0F;
			checkSum ^= pAtr->FieldBuf[i];
			y =  pAtr->FieldBuf[i++] & 0xF0;
		}
		else if (!(y & 0x80)) {
			y = 0;
			if (offset==1) {
				pAtr->ATRDecResult[ATRProtocolType] = PROTOCOL_TYPE_T0;
			}
        }
        offset++;
	}

	// Historical Bytes
	y = pAtr->FieldBuf[1] & 0x0F;
	for( j=0; j < y; j++ ) {
		if_dev_get_char(&pAtr->FieldBuf[i]);
		checkSum ^= pAtr->FieldBuf[i++];
	}

	// TCK
	if (pAtr->ATRDecResult[ATRProtocolType]) {
		if_dev_get_char(&pAtr->FieldBuf[i++]);
			printk("TCK byte exist\n");
		if (pAtr->FieldBuf[i] != checkSum) {
			printk("TCK check sum error, failed to get ATR\n");
			return -1;
		}
	}

	dbg_print("Length = %d\n", i);
	dbg_print("ATR = ");
    for (j=0; j < i; j++) {
        dbg_print("%02x ", pAtr->FieldBuf[j]);
    }
    dbg_print("\n");

    pAtr->len = i;

    return 0;
}

static int card_decode_ATR( ATRField* pAtr )
{
    unsigned int i;
    unsigned int j;
    unsigned int y;
    unsigned char offset;

    dbg_print("ATR: Answer To Reset:\n");
    dbg_print("TS = 0x%X Initial character ", pAtr->FieldBuf[0]);
    snprintf((char *)(&pAtr->FieldName[0]), ATR_FILED_NAME_SIZE, "TS");
    if( pAtr->FieldBuf[0] == 0x3B ) 
	{
        pAtr->ATRDecResult[inverse_logic] = 0;
        card_logic_mode = DIRECT_MODE;
        dbg_print("Direct Convention\n");
    }
    else if( pAtr->FieldBuf[0] == 0x3F ) 
	{
        pAtr->ATRDecResult[inverse_logic] = 1;
        card_logic_mode = INVERSE_MODE;
	    dbg_print("Inverse Convention\n");
    }
    else {
        printk("failed to decode ATR\n");
        return -1;
    }

    dbg_print("T0 = 0x%X Format character\n",pAtr->FieldBuf[1]);
    snprintf((char *)(&pAtr->FieldName[1]), ATR_FILED_NAME_SIZE, "T0");
    dbg_print("    Number of historical bytes: K = %d\n", pAtr->FieldBuf[1] & 0x0F);
    pAtr->ATRDecResult[num_historical_bytes] = pAtr->FieldBuf[1]& 0x0F;
    dbg_print("    Presence further interface byte:\n");
    if( pAtr->FieldBuf[1] & 0x80 ) {
        pAtr->ATRDecResult[TD_exist] = 1;
        dbg_print("TD ");
    }
    else if (!( pAtr->FieldBuf[1] & 0x80 )) {
        pAtr->ATRDecResult[ATRProtocolType] = PROTOCOL_TYPE_T0;
    }
    if( pAtr->FieldBuf[1]& 0x40 ) {
        pAtr->ATRDecResult[TC_exist] = 1;
        dbg_print("TC ");
    }
    if( pAtr->FieldBuf[1] & 0x20 ) {
        pAtr->ATRDecResult[TB_exist] = 1;
        dbg_print("TB ");
    }
    if( pAtr->FieldBuf[1] & 0x10 ) {
        pAtr->ATRDecResult[TA_exist] = 1;
        dbg_print("TA ");
    }
    if( pAtr->FieldBuf[1]!= 0 ) {
        dbg_print(" present\n");
    }

    i = 2;
    y = pAtr->FieldBuf[1] & 0xF0;

    // Read ATR Ti
    offset = 1;
    while (y) {

        if (y & 0x10) {  // TA[i]
            dbg_print("TA[%d] = 0x%02X\n", offset, pAtr->FieldBuf[i]);
            snprintf((char *)(&pAtr->FieldName[i]), ATR_FILED_NAME_SIZE, "TA[%d]", offset);
            if( offset == 1 ) {
                switch(((pAtr->FieldBuf[i])>>4) & 0x0F) {
                case 0x00:
                case 0x01:
                    pAtr->ATRDecResult[ATR_FI_decimal] = 372;
                    break;
                case 0x02:
                    pAtr->ATRDecResult[ATR_FI_decimal] = 558;
                    break;
                case 0x03:
                    pAtr->ATRDecResult[ATR_FI_decimal] = 744;
                    break;
                case 0x04:
                    pAtr->ATRDecResult[ATR_FI_decimal] = 1116;
                    break;
                case 0x05:
                    pAtr->ATRDecResult[ATR_FI_decimal] = 1488;
                    break;
                case 0x06:
                    pAtr->ATRDecResult[ATR_FI_decimal] = 1860;
                    break;
                case 0x09:
                    pAtr->ATRDecResult[ATR_FI_decimal] = 512;
                    break;
                case 0x0A:
                    pAtr->ATRDecResult[ATR_FI_decimal] = 768;
                    break;
                case 0x0B:
                    pAtr->ATRDecResult[ATR_FI_decimal] = 1024;
                    break;
                case 0x0C:
                    pAtr->ATRDecResult[ATR_FI_decimal] = 1536;
                    break;
                case 0x0D:
                    pAtr->ATRDecResult[ATR_FI_decimal] = 2048;
                    break;
                default:
                    printk("unknown FI parse from ATR: 0x%02x\n", (((pAtr->FieldBuf[i])>>4) & 0x0F));
                       // return -1;
                    break;
                }
                dbg_print("FI = %d ", pAtr->ATRDecResult[ATR_FI_decimal]);

                switch(pAtr->FieldBuf[i] & 0x0F) {
                case 0x01:
                    pAtr->ATRDecResult[ATR_DI_decimal] = 1;
                    break;
                case 0x02:
                    pAtr->ATRDecResult[ATR_DI_decimal] = 2;
                    break;
                case 0x03:
                    pAtr->ATRDecResult[ATR_DI_decimal] = 4;
                    break;
                case 0x04:
                    pAtr->ATRDecResult[ATR_DI_decimal] = 8;
                    break;
                case 0x05:
                    pAtr->ATRDecResult[ATR_DI_decimal] = 16;
                    break;
                case 0x06:
                    pAtr->ATRDecResult[ATR_DI_decimal] = 32;
                    break;
                case 0x08:
                    pAtr->ATRDecResult[ATR_DI_decimal] = 12;
                    break;
                case 0x09:
                    pAtr->ATRDecResult[ATR_DI_decimal] = 20;
                    break;
                default:
                    printk("unknown DI parse from ATR: 0x%02x\n", (pAtr->FieldBuf[i] & 0x0F));
                       // return -1;
                    break;
                }
                dbg_print("DI = %d ", pAtr->ATRDecResult[ATR_DI_decimal]);
            }
            if( offset == 2 ) {
                pAtr->ATRDecResult[ATRProtocolType] |= pAtr->FieldBuf[i]&0x0F;
            }
            i++;
        }
        if (y & 0x20) {  // TB[i]
            dbg_print("TB[%d] = 0x%X\n", offset, pAtr->FieldBuf[i]);
            snprintf((char *)(&pAtr->FieldName[i]), ATR_FILED_NAME_SIZE, "TB[%d]", offset);
            i++;
        }
        if (y & 0x40) {  // TC[i]
            dbg_print("TC[%d] = 0x%X ", offset, pAtr->FieldBuf[i]);
            snprintf((char *)(&pAtr->FieldName[i]), ATR_FILED_NAME_SIZE, "TC[%d]", offset);
            if( offset == 1 ) {
                dbg_print("Extra Guard Time: N = %d", pAtr->FieldBuf[i]);
                pAtr->ATRDecResult[Extra_Guard_Time] = pAtr->FieldBuf[i];
            }
            dbg_print("\n");
            i++;
        }
        if (y & 0x80) {  // TD[i]
            dbg_print("TD[%d] = 0x%X\n", offset, pAtr->FieldBuf[i]);
            snprintf((char *)(&pAtr->FieldName[i]), ATR_FILED_NAME_SIZE, "TD[%d]", offset);
            pAtr->ATRDecResult[ATRProtocolType] |= pAtr->FieldBuf[i]&0x0F;
            y = pAtr->FieldBuf[i++] & 0xF0;
        }
        else if(!(y & 0x80)) {
            y = 0;
        }
        offset++;
    }

    // Historical Bytes
    dbg_print("Historical bytes:\n");
    y = pAtr->FieldBuf[1] & 0x0F;
    for( j=0; j < y; j++ ) {

        dbg_print(" 0x%X", pAtr->FieldBuf[i]);
        snprintf((char *)(&pAtr->FieldName[i]), ATR_FILED_NAME_SIZE, "T[%d]", j);
        if((pAtr->FieldBuf[i] > 0x21) && (pAtr->FieldBuf[i] < 0x7D) ) {  // ASCII
            dbg_print("(%c) ", pAtr->FieldBuf[i]);
        }
        i++;
    }
    dbg_print("\n");

    // TCK 
    if (pAtr->ATRDecResult[ATRProtocolType]) {
        dbg_print("TCK : 0x%X\n", pAtr->FieldBuf[i]);
        snprintf((char *)(&pAtr->FieldName[i]), ATR_FILED_NAME_SIZE, "TCK");
    }
    return 0;
}

static int card_set_protocol(unsigned char protocol)
{
    protocolType = protocol;

    if (protocol==PROTOCOL_TYPE_T1) {
        *US_MR |= AT91C_US_USMODE_ISO7816_1;
    } else if (protocol==PROTOCOL_TYPE_T0) {
        *US_MR |= AT91C_US_USMODE_ISO7816_0;
    } else {
        printk("unsupported protocol type\n");
        return -1;
    }
    dbg_print("set protocol: T=%d\n", protocol);

    return 0;
}

static int card_send_APDU(unsigned char *send_buf, int send_len, unsigned char *recv_buf, int *recv_len)
{
    int retval;
    int i;
    int count;
    unsigned char tmp[5];
    unsigned char  *ptr;
    unsigned char  INS;
    unsigned char  first = 1;
    int APDU_Lc;
    int APDU_Le;

    *recv_len = 0;

        if (send_len<4)  {
            dbg_print("@%d card_send_APDU error\n", __LINE__);
            return APDU_ERR;
        }

        INS = *(send_buf+1);
    ptr = send_buf;

        if (send_len==4) {
            tmp[0] = *(send_buf);
            tmp[1] = *(send_buf+1);
            tmp[2] = *(send_buf+2);
            tmp[3] = *(send_buf+3);
            tmp[4] = 0;
            APDU_Le = tmp[4];
            APDU_Lc = -1;
        }
        else if (send_len==5) {
            APDU_Le = *(send_buf+4);
            APDU_Lc = -1;
        }
        else{
            APDU_Lc = *(send_buf+4);
            if (send_len == APDU_Lc+5) {
                APDU_Le = -1;
            }
            else if (send_len == APDU_Lc+5+1) {
                APDU_Le = *( send_buf+send_len -1);
            }
            else {
                return APDU_ERR;
            }
        }

    if ( ( APDU_Lc == -1 ) && (APDU_Le == -1) ) {
        dbg_print("@%d card_send_APDU error\n", __LINE__);
        return APDU_ERR;
    }
    if ( ( APDU_Lc >= 256 ) || (APDU_Le >= 256) ) {
        dbg_print("@%d card_send_APDU error\n", __LINE__);
        return APDU_ERR;
    }
    if ( ( APDU_Lc == 0 ) ) {
        dbg_print("@%d card_send_APDU error\n", __LINE__);
        return APDU_ERR;
    }

    for (i = 0; i < 5; i++)
        {
        if (if_dev_send_char(*ptr++))
                {
            dbg_print("@%d card_send_APDU error\n", __LINE__);
            return -1;
        }
    }

    ptr = recv_buf;
    if ( APDU_Lc != -1 )
        {
        while (1)
        {
            if (if_dev_get_char(ptr)) {
                *recv_len = 0;
                dbg_print("@%d card_send_APDU error\n", __LINE__);
                return -1;
            }
            if (!(*ptr==0x60) ){
                break;
            }
        }
        count = 0;
        ptr = send_buf+5;
        if ( ( recv_buf[0]==INS )||( recv_buf[0]==(INS^0x01) ) )
                {
            udelay(400);
            for (i = 0; i < APDU_Lc; i++)
                        {
                if (if_dev_send_char(*ptr++)) {
                    dbg_print("@%d card_send_APDU error\n", __LINE__);
                    return -1;
                }
            }

            ptr = recv_buf;
            first = 1;
            if ( APDU_Le == 0 )
                        {
                while (1)
                {
                    retval = if_dev_get_char(ptr);
                    if ( retval==TIMEROUT_ERR)
                    {
                        dbg_print("@%d card_send_APDU error\n", __LINE__);
                        return -1;
                    }
                    if ( retval<0 ) {
                        *recv_len = 0;
                        dbg_print("@%d card_send_APDU error\n", __LINE__);
                        return -1;
                    }
                    if ( !((*ptr==0x60)&&(first==1)) ){
                        first=0;
                        ptr++;
                        (*recv_len)++;
                        if ( *recv_len >= 256 ) {
                            return (send_len);
                        }
                    }
                }
            }
            else if ( APDU_Le == -1 )
                        {
                while (1){
                    if (if_dev_get_char(ptr)) {
                        *recv_len = 0;
                        dbg_print("@%d card_send_APDU error\n", __LINE__);
                        return -1;
                    }
                    if (!((*ptr==0x60)&&(first==1))) {
                        first = 0;
                        ptr++;
                        (*recv_len)++;
                        if (*recv_len >= 2) {
                            return (send_len);
                        }
                    }
                }
            }
            else{
                while (1){
                    if (if_dev_get_char(ptr)) {
                        *recv_len = 0;
                        dbg_print("@%d card_send_APDU error\n", __LINE__);
                        return -1;
                    }
                    if (!((*ptr==0x60)&&(first==1))){
                        first = 0;
                        ptr++;
                        (*recv_len)++;
                        if (*recv_len >= APDU_Le+2) {
                            return (send_len);
                        }
                    }
                }
            }

        }
        else if( ( recv_buf[0]==(INS^0xff) )||( recv_buf[0]==(INS^0xfe) ) ){
repeat:
            udelay(400);
            if (if_dev_send_char(*ptr)) {
                dbg_print("@%d card_send_APDU error\n", __LINE__);
                return -1;
            }

            first = 1;
            while (1){
                retval = if_dev_get_char(ptr);
                if ( retval==TIMEROUT_ERR) {
                    dbg_print("@%d card_send_APDU error\n", __LINE__);
                    return -1;
                }
                if ( retval<0 ) {
                    *recv_len = 0;
                    dbg_print("@%d card_send_APDU error\n", __LINE__);
                    return -1;
                }
                if (!((*ptr==0x60)&&(first==1))) {
                    if((( *ptr==(INS^0xff) )||( *ptr==(INS^0xfe)))&&(count<APDU_Lc-1)) {
                        count++;
                        ptr = send_buf+5+count;
                        if(count >= APDU_Lc-1) {
                            ptr = recv_buf;
                        }
                        goto repeat;
                    }
                    first = 0;
                    ptr++;
                    (*recv_len)++;
                    if (*recv_len >= 2) {
                        return (send_len);
                    }
                }
            }
        }
        else {
            ptr++;
            (*recv_len)++;
            if (if_dev_get_char(ptr)) {
                *recv_len = 0;
                dbg_print("@%d card_send_APDU error\n", __LINE__);
                return -1;
            }
            ptr++;
            (*recv_len)++;
            return (send_len);
        }
    }
    else
        {
        while (1)
        {
            if (if_dev_get_char(ptr)) {
                *recv_len = 0;
                dbg_print("@%d card_send_APDU error\n", __LINE__);
                return -1;
            }
            if (!(*ptr==0x60)) {
                break;
            }
        }
        if ( ( recv_buf[0]==INS )||( recv_buf[0]==(INS^0x01) ) ) {
            if ( APDU_Le == 0 ) {
                while (1)
                {
                    retval = if_dev_get_char(ptr);
                    if (retval==TIMEROUT_ERR)
                    {
                        dbg_print("@%d card_send_APDU error\n", __LINE__);
                        return -1;
                    }
                    if (retval < 0) {
                        *recv_len = 0;
                        dbg_print("@%d card_send_APDU error\n", __LINE__);
                        return -1;
                    }
                    ptr++;
                    (*recv_len)++;
                    if ( *recv_len >= 256 ) {
                        return (send_len);
                    }
                }
            }
            else {
                while (1)
                {
                    retval = if_dev_get_char(ptr);
                    if (retval < 0) {
                        *recv_len = 0;
                        dbg_print("@%d card_send_APDU error\n", __LINE__);
                        return -1;
                    }
                    if (*recv_len >= APDU_Le) {
                        if ((*ptr != 0x60)||(*(ptr-1)==0x90)) {
                            ptr++;
                            (*recv_len)++;
                        }
                    }
                    else {
                        ptr++;
                        (*recv_len)++;
                    }
                    if ( *recv_len >= APDU_Le+2 ) {
                        return (send_len);
                    }
                }
            }
        }
        else if ( ( recv_buf[0]==(INS^0xff) )||( recv_buf[0]==(INS^0xfe) ) ) {
            while (1)
            {
                retval = if_dev_get_char(ptr);
                if (retval==TIMEROUT_ERR) {
                    dbg_print("@%d card_send_APDU error\n", __LINE__);
                    return -1;
                }
                if (retval < 0) {
                    *recv_len = 0;
                    dbg_print("@%d card_send_APDU error\n", __LINE__);
                    return -1;
                }
                if (( *ptr!=(INS^0xff) )&&( *ptr!=(INS^0xfe))) {
                    ptr++;
                    (*recv_len)++;
                    if ( *recv_len >= APDU_Le+2 ) {
                        return (send_len);
                    }
                }
            }
        }
        else {
            ptr++;
            (*recv_len)++;
            if (if_dev_get_char(ptr)) {
                *recv_len = 0;
                dbg_print("@%d card_send_APDU error\n", __LINE__);
                return -1;
            }
            ptr++;
            (*recv_len)++;
            return (send_len);
        }
    }

#if 0
printk("card_send_APDU() return\n");
return send_len;
#endif

}

static int sam_open(struct inode *inode, struct file *file)
{
    dbg_print("\n[Kernel] Open Smart Card.\n");
    spin_lock(&spin);
    if (dev_open_count) {
        spin_unlock(&spin);
        return -EBUSY;
    }

#ifdef PLAT_L2
   // printk("SAM CARD present pin level: %d\n", at91_get_gpio_value (SAM_CHK));
    if (HIGHLEVEL == at91_get_gpio_value (SAM_CHK))
    {
        printk("SAM card not present so SAM open() failure.\n"); 
        return -ENODEV;
    }
#endif

    dev_open_count++;
    spin_unlock(&spin);

    memset((void *)&ATRBuf, 0, sizeof(ATRField));

    card_activate();
    card_cold_reset(); 

    if (card_get_ATR(&ATRBuf))
        {
        printk("failed to get ATR\n");
        card_deactivate();
                dev_open_count--;
        return -1;
    }

    if (card_decode_ATR(&ATRBuf))
        {
        printk("failed to decode ATR\n");
        card_deactivate();
        dev_open_count--;
        return -1;
    }
    if (card_set_protocol(ATRBuf.ATRDecResult[ATRProtocolType]))
        {
        printk("failed to set protocol\n");
        card_deactivate();
                dev_open_count--;
        return -1;
    }

    //*US_FIDIR = (ATRBuf.ATRDecResult[ATR_FI_decimal]/ATRBuf.ATRDecResult[ATR_DI_decimal]);/* modified on 12-23 */
    //print("FI/DI = %d\n", *US_FIDIR);
    dbg_print("sam_open() ok\n");

    return 0;
}

static int sam_release(struct inode *inode, struct file *file)
{
    card_deactivate();
    dev_open_count = 0;
    dbg_print("sam_release() ok\n");

    return 0;
}

static int sam_write (struct file *file, const char *buf, size_t len, loff_t *offset)
{
    int i;
    int retval = 0;
    char kbuf[CARD_BUF_SIZE];

	dbg_print("\n[Kernel] Write Data to Smart Card.\n");

    //print("sam_write() called\n");

    if ((len>=sizeof(kbuf)) || (len<=0)) {
        dbg_print("sam_write(): len error\n");
        return -EINVAL;
    }

    dbg_print("APDU data from user space: ");
        for (i=0; i < len; i++) {
                dbg_print("0x%02X ", buf[i]);
        }
        dbg_print("\n");
    memset(kbuf, 0, sizeof(kbuf));
    if (copy_from_user((void *)kbuf, (const void *)buf, len)) {
        return -EFAULT;
    }

    memset(&APDUAnswerBuf, 0, sizeof(APDUAnswerBuf));
    retval = card_send_APDU((unsigned char *)kbuf, len, APDUAnswerBuf.buf, &(APDUAnswerBuf.len));

    return retval;
}


static int sam_read (struct file *file, char *buf, size_t len, loff_t *offset)
{
    int retval = 0;

    //print("sam_read() called\n");

    dbg_print("\n[Kernel] Read Data from Smart Card.\n");
    if (APDUAnswerBuf.len==0) 
	{
        return 0;
    }

    retval = APDUAnswerBuf.len;
    if ( copy_to_user ((void *)buf, (void *)APDUAnswerBuf.buf, APDUAnswerBuf.len) ) 
	{
        return -EFAULT;
    }
    APDUAnswerBuf.len = 0;

    return retval;
}

static long sam_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret;
    switch (cmd)
    {
		case SET_DRV_DEBUG:
		  dbg_print("%s driver debug now.\n", DISABLE == arg ? "Disable" : "Enable");

		  if (0 == arg)
		      debug = DISABLE;
		  else
		      debug = ENABLE;

		  break;

		case GET_DRV_VER:
		  print_version(DRV_VERSION);
		  return DRV_VERSION;

		case SAM_SEL_VCC_TYPE:
		    m_ucVccType = (unsigned char)arg;
		    if (m_ucVccType!=VccType3V3 && m_ucVccType!=VccType5V) 
			{
		        printk("ioctl SELECT_CARD_VCC_TYPE failed, unsupported vcc type\n");
		        card_deactivate();
		        return -1;
		    }
		    card_set_power(m_ucVccType);
		    dbg_print("ioctl SELECT_CARD_VCC_TYPE ok, vcc type: %sV\n", ((m_ucVccType==VccType3V3) ? "3.3" : "5"));
		    break;

	    case SAM_POWER_UP:
			dbg_print("\n[Kernel] Power up Card.\n");
            card_activate();
            card_cold_reset();
            if (card_get_ATR(&ATRBuf))
            {
                printk("failed to get ATR\n");
                card_deactivate();
                return -1;
            }
            if (card_decode_ATR(&ATRBuf))
            {
                printk("failed to decode ATR\n");
                card_deactivate();
                return -1;
            }
            if (card_set_protocol(ATRBuf.ATRDecResult[ATRProtocolType]))
            {
                printk("failed to set protocol\n");
                card_deactivate();
                return -1;
            }
	        break;

		case SAM_CARD_RESET:
			dbg_print("\n[Kernel] Reset Card.\n");
		    card_hot_reset();
		    memset((void *)&ATRBuf, 0, sizeof(ATRField));
		    if (card_get_ATR(&ATRBuf)) {
		        printk("ioctl CARD_RESET failed, get ATR error\n");
		        card_deactivate();
		        return -1;
		    }
		    if (card_decode_ATR(&ATRBuf)) {
		        printk("ioctl CARD_RESET failed, decode ATR error\n");
		        card_deactivate();
		        return -1;
		    }
		    if (card_set_protocol(ATRBuf.ATRDecResult[ATRProtocolType])) {
		        printk("ioctl CARD_RESET failed, set protocol error\n");
		        card_deactivate();
		        return -1;
			}
		    dbg_print("ioctl CARD_RESET ok\n");
		    break;

		case SAM_CARD_STATUS:
			dbg_print("\n[Kernel] Get Card Status\n");
		    if (copy_to_user((void *)arg, (const void *)(&ATRBuf), sizeof(ATRField))) {
		        printk("ioctl CARD_STATUS failed\n");
		        card_deactivate();
		        return -1;
		    }
		    dbg_print("ioctl CARD_STATUS ok\n");
		    break;

		case SAM_POWER_OFF: /*The same as CARD_DEACTIVE*/
		    dbg_print("\n[Kernel] Power off Card\n");
		    card_deactivate();
		    dbg_print("ioctl POWER_OFF ok\n");
		    break;

		case SAM_CARD_ACTIVE:
		    card_activate();
		    break;

		case SAM_SET_PARA:
		    break;

		case SAM_SET_PROT:
		    break;		  

		case SAM_PRESENT_DETECT:
#ifdef PLAT_L2
            if (LOWLEVEL == at91_get_gpio_value (SAM_CHK))
                   ret = CARD_PRESENT;
            else
                   ret = CARD_NOT_PRESENT;
#else /*L3 doesn't support GPIO to get card present status, it should always return card PRESENT*/
                   ret = CARD_PRESENT;
#endif
            put_user(ret, (int *)arg);
            return ret;

		default:
			printk("%s driver don't support ioctl command=%d\n", DEV_NAME, cmd);
			return -1;
    }

    return 0;
}

static struct file_operations sam_fops = {
    .owner = THIS_MODULE,
    .open = sam_open,
    .release = sam_release,
    .read = sam_read,
    .write = sam_write,
    .unlocked_ioctl = sam_ioctl,
};

static void sam_hw_init(void)
{
    at91_set_B_periph(SAM_SCK, DISPULLUP);
    at91_set_gpio_output(SAM_RST, HIGHLEVEL);
    at91_set_A_periph(SAM_DATA, DISPULLUP);
    at91_set_gpio_output(SAM_M0, LOWLEVEL);
    at91_set_gpio_output(SAM_M1, LOWLEVEL);
#ifdef PLAT_L2
    at91_set_gpio_output(SAM_M2, LOWLEVEL);
    at91_set_gpio_input(FAKE_SAM_M0, DISPULLUP);
    at91_set_gpio_input(FAKE_SAM_M1, DISPULLUP);
    at91_set_gpio_input(FAKE_SAM_M2, DISPULLUP);
    at91_set_gpio_input (SAM_CHK, DISPULLUP);
#endif

    US_CR   = ioremap(AT91SAM9260_BASE_US3 + 0x00, 0x04); /*Control Register*/
    US_MR   = ioremap(AT91SAM9260_BASE_US3 + 0x04, 0x04); /*Mode Register*/
    US_CSR  = ioremap(AT91SAM9260_BASE_US3 + 0x14, 0x04); /*Interrupt Enable Register*/
    US_RHR  = ioremap(AT91SAM9260_BASE_US3 + 0x18, 0x04); /*Receiver Holding Register*/
    US_THR  = ioremap(AT91SAM9260_BASE_US3 + 0x1c, 0x04); /*Transmitter Holding Register*/
    US_IDR  = ioremap(AT91SAM9260_BASE_US3 + 0x0c, 0x04); /*Interrupt Disable Register*/
    US_TTGR = ioremap(AT91SAM9260_BASE_US3 + 0x28, 0x04); /*Transmitter Timeguard Register*/
    US_BRGR = ioremap(AT91SAM9260_BASE_US3 + 0x20, 0x04); /*Baud Rate Generator Register*/
    US_FIDIR= ioremap(AT91SAM9260_BASE_US3 + 0x40, 0x04); /*FI DI Ratio Register*/
    US_NER  = ioremap(AT91SAM9260_BASE_US3 + 0x44, 0x04); /*Number of Error Register*/
    US_RTOR = ioremap(AT91SAM9260_BASE_US3 + 0x24, 0x04); /*Receiver Time-out Register */

    /* 
     * Reset and disable receiver & transmitter
     * RSTRX(1): Resets the receiver  RSTTX(1): Resets the Transmitter 
     * RXDIS(1): Disable the receiver TXDIS(1): Disable the transmitter 
     */
    *US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RXDIS | AT91C_US_TXDIS;

    *US_MR =  AT91C_US_USMODE_ISO7816_0
                     | AT91C_US_CLKS_CLOCK
                     | AT91C_US_NBSTOP_1_BIT
                     | AT91C_US_PAR_EVEN
                     | AT91C_US_CHRL_8_BITS  /*  AT91C_US_MODE9 */
                     | AT91C_US_CKLO
                     | (3<<24); // MAX_ITERATION
    *US_BRGR = 0;

    at91_sys_write (AT91_PMC_PCER, 1 << AT91SAM9260_ID_US3);
    *US_IDR = (unsigned int) -1;
    *US_FIDIR = 372;  // by default 
    at91_get_clock(18432000); /*18.432 MHz crystal, used to get mck_rate_hz*/
    /*
     * printk("Original MCK: %u New MCK:%lu\n", BOARD_MCK, mck_rate_hz);
     * L350:  Original MCK: 99328000 New MCK:198656000
     * L2:    132096000
     */
    printk("Master Clock:%lu\n", mck_rate_hz);
	*US_BRGR = mck_rate_hz / (372*9600);
//	*US_BRGR = BOARD_MCK / (372*9600);
	*US_TTGR = 0;/* modified on 12-22 */
	*US_RTOR = 65535;
}

static void sam_hw_term(void)
{
    iounmap (US_CR);
    iounmap (US_MR);
    iounmap (US_CSR);
    iounmap (US_RHR);
    iounmap (US_THR);
    iounmap (US_IDR);
    iounmap (US_TTGR);
    iounmap (US_BRGR);
    iounmap (US_FIDIR);
    iounmap (US_NER);
    iounmap (US_RTOR);
}

static void sam_cleanup(void)
{
    dev_t devno = MKDEV(dev_major, dev_minor);

	sam_hw_term();

    device_destroy(dev_class, devno);   
    class_destroy(dev_class);

    cdev_del(dev_cdev);
    unregister_chrdev_region(devno, 1);

    printk("%s driver removed\n", DEV_NAME);
}

static int __init sam_init(void)
{
    int result;
    dev_t devno;

	/*Init sam hardware*/
	sam_hw_init();

    /*Alloc for the device for driver */
    if (0 != dev_major)
    {
        devno = MKDEV(dev_major, dev_minor);
        result = register_chrdev_region(devno, 1, DEV_NAME);
    }
    else
    {
        result = alloc_chrdev_region(&devno, dev_minor, 1, DEV_NAME);
        dev_major = MAJOR(devno);
    }

    /*Alloc for device major failure */
    if (result < 0)
    {
        printk("%s driver can't get major %d\n", DEV_NAME, dev_major);
        return result;
    }

    /*Alloc cdev structure */
    dev_cdev = cdev_alloc();;
    if (NULL == dev_cdev)
    {
		unregister_chrdev_region(devno, 1);
        printk("%s driver can't alloc for sam_cdev\n", DEV_NAME);
        return -ENODEV;
    }

    /*Initialize cdev structure and register it */
    dev_cdev->owner = THIS_MODULE;
    dev_cdev->ops = &sam_fops;
    result = cdev_add(dev_cdev, devno, 1);
    if (0 != result)
    {
        printk("%s driver can't alloc for sam_cdev\n", DEV_NAME);
        goto ERROR;
    }

	dev_class = class_create(THIS_MODULE, DEV_NAME);
    if(IS_ERR(dev_class)) 
    {           
        printk("%s driver create class failture\n",DEV_NAME);           
        result =  -ENOMEM;  
        goto ERROR;   
    }       

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)     
    device_create(dev_class, NULL, devno, NULL, DEV_NAME);
#else
    device_create (dev_class, NULL, devno, DEV_NAME);
#endif  
    
    printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER,
               DRV_REVER_VER);
	return 0;

ERROR:
    cdev_del(dev_cdev);
    unregister_chrdev_region(devno, 1);
    return result;
}

module_init(sam_init);
module_exit(sam_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRV_DESC);

