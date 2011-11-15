/*********************************************************************************
 *      Copyright:  (C) 2011 R&D of San Fran Electronics Co., LTD  
 *                  All rights reserved.
 *
 *       Filename:  gpio_gstty.c
 *    Description:  This a GPIO Simulate UART driver for the IrDA communication
 *                 
 *        Version:  1.0.0(10/27/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "10/27/2011 11:39:10 AM"
 *
 *        TestLog:  2011.11.12 
 *           1, Baudrata: 9600bps is not stable when send '0123456789' in period 100ms. 
 *           2, Baudrata: 1200bps bit data error.
 *                 
 ********************************************************************************/

#include "at91_driver.h"

#include <mach/at91_tc.h>
#include <linux/atmel_tc.h>
#include <mach/at91_pio.h>
#include <linux/clk.h>
#include <linux/wait.h>

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9XXX GPIO Simulator UART driver"

/*  Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_GSTTY_NAME

//#define DEV_MAJOR                 DEV_GSTTY_MAJOR
#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*   dynamic major by default */ 
#endif

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;

static int baudrate = 1200;

static struct cdev cdev;
static struct class *dev_class = NULL;

#define AT91_TCBLOCK0             0 /* Time Counter Block 0, for TC0,TC1,TC2  */
#define AT91_TCBLOCK1             1 /* Time Counter Block 1, for TC3,TC4,TC5  */
#define AT91_TC_CHN0              0 /* Channel 0 in a TC Block */
#define AT91_TC_CHN1              1 /* Channel 1 in a TC Block */
#define AT91_TC_CHN2              2 /* Channel 2 in a TC Block */

#define RXD_CLOCK_CHN             AT91_TC_CHN0
#define TXD_CLOCK_CHN             AT91_TC_CHN1

#define RXD_CLOCK_IRQ             AT91SAM9260_ID_TC0
#define TXD_CLOCK_IRQ             AT91SAM9260_ID_TC1

#define TXD_GPIO_PIN              AT91_PIN_PB4
#define RXD_GPIO_PIN              AT91_PIN_PB5

#define CIRC_BUF_SIZE             1024

static void __iomem    *tcaddr;
struct atmel_tc        *tc; 

struct circ_buf        rx_ring;
struct circ_buf        tx_ring;

static DECLARE_WAIT_QUEUE_HEAD(tx_waitq);

/* Start the TC channel to provide the receive/send data clock */
static inline void start_tc(int channel, unsigned long baudrate)
{
    unsigned long mck_rate_hz = 0;

    /* Enable the TC Clock in AT91_PMC_PCER(Can Read status from AT91_PMC_PCSR) */
    clk_enable(tc->clk[channel]);

    /* Get the MCK */
    mck_rate_hz = clk_get_rate(tc->clk[channel]);

    /* Clock Disable */
    __raw_writel(AT91_TC_CLKDIS, tcaddr + ATMEL_TC_REG(channel, CCR));  

    /* All interrupt disable */
    __raw_writel(0xFF, tcaddr + ATMEL_TC_REG(channel, IDR));  

    /* Choose External Clock1(MCK/2),RC Compare triger */ 
    __raw_writel(AT91_TC_TIMER_CLOCK1| AT91_TC_CPCTRG, tcaddr + ATMEL_TC_REG(channel, CMR));  

    dbg_print("mck_rat_hz=%lu baudrate=%lu RC=%lu\n", mck_rate_hz, baudrate, mck_rate_hz/(2*baudrate));
    /* Set the counter value */
    __raw_writel((mck_rate_hz/(2*baudrate)), tcaddr + ATMEL_TC_REG(channel, RC));  

    /* RC compare interrupt enable */ 
    __raw_writel((1<<4), tcaddr + ATMEL_TC_REG(channel, IER));  
    
    /* TC Channel clock enable */
    __raw_writel((AT91_TC_CLKEN), tcaddr + ATMEL_TC_REG(channel, CCR));  

    /* Clear the TC Channel status  */
    __raw_readl(tcaddr + ATMEL_TC_REG(channel, SR));

    /* TC channel software trigger enable */
    __raw_writel((AT91_TC_SWTRG ), tcaddr + ATMEL_TC_REG(channel, CCR));  
}

/* Stop the TC channel  */
static void stop_tc(int channel)
{
    /* Disable the TC Clcok */
    clk_disable(tc->clk[channel]);

    /* Disable the RC Compare interrupt  */
    __raw_writel((1<<4), tcaddr + ATMEL_TC_REG(channel, IDR));  

    /* Disable the RC C  */
    __raw_writel(AT91_TC_CLKDIS, tcaddr + ATMEL_TC_REG(channel, CCR));  
}

/* Read the TC_SR register will clear the TC */
static void clear_tc(int channel)
{ 
    __raw_readl(tcaddr + ATMEL_TC_REG(channel, SR));
}

#define START_BIT          0
#define STOP_BIT           9
static irqreturn_t txdtc_interrupt_handler(int irq, void *dev_id)
{
    static int         i = 0;
    static char        bits[10];
    static char        in_byte = 0x00;
    int                j=0;

    clear_tc(TXD_CLOCK_CHN);

    /* The send circle buffer get data need to be send */
    if( in_byte ||  CIRC_CNT(tx_ring.head, tx_ring.tail, CIRC_BUF_SIZE)>=1 )
    {
        //printk("in_byte=%d size=%d\n", in_byte, CIRC_CNT(tx_ring.head, tx_ring.tail, CIRC_BUF_SIZE));
        /* A byte data include 1 start bit, 1 stop bit, 8 data bit */
        if(0 == (i%10) )
        {
            i=0;
            in_byte = 0x01;

            bits[START_BIT]=LOWLEVEL;  /* Start bit */
            for(j=0; j<8; j++)       
                bits[j+1]=(tx_ring.buf[tx_ring.tail]>>j) & 0x01;  /* Data bit */
            bits[STOP_BIT]=HIGHLEVEL;  /* Stop bit  */
        }

        at91_set_gpio_value(AT91_PIN_PB4, bits[i]);
        if(STOP_BIT==i)
        {
            tx_ring.tail = (tx_ring.tail + 1) & (CIRC_BUF_SIZE - 1); 
            in_byte = 0x00;
        }

        i++;
    }
    else
        wake_up_interruptible(&tx_waitq);

    return IRQ_HANDLED;
}


/*==============================================================================================
 *  RXD PIN DATA WAVE:
 *
 *              bit0        bit2      bit4
 *  ____     ________     _______     ____
 *     |     |      |     |     |     |
 *     |_____|      |_____|     |_____|
 *    StartBit        bit1        bit3
 *
 * TC Sample Clock:
 *
 *          1     3     5      7      9
 *    __  _____  ____  ____   ____  ____
 *     |  |   |  |  |  |  |   |  |  |
 *     |__|   |__|  |__|  |___|  |__|
 *      0      2     4     6       8
 *
 *  TC clock0: It's in RXD start bit circle, we shouldn't use it;
 *  TC clock1: It's uncertain about I'm in data StartBit or Bit[0], so don't use it.
 *  TC clock2: It's should in data bit[0] circlue, sample the data;
 *  TC clock3: It's uncertain about I'm in data bitp[0] or Bit[1], so don't use it.
 *  TC clock4: It's should in data bit[1] circlue, sample the data;
 *  TC clock5: It's uncertain about I'm in data bitp[1] or Bit[2], so don't use it.
 *  ......
 *
 *=============================================================================================*/ 

/* This is the Receive Time Counter interrupt handler, which used to provide receive
 * clock and receive one bit in a clock */
static irqreturn_t rxdtc_interrupt_handler(int irq, void *dev_id)
{
    static int             i=0;
    unsigned int           status = 0;
    static unsigned  char  ch = 0;

    clear_tc(RXD_CLOCK_CHN);

    //status = (at91_sys_read(AT91_PIOB+PIO_PDSR)>>5)&0x01;
    status = at91_get_gpio_value(AT91_PIN_PB5);

    /* Skip sample Clock1, clock3, clock5, clock7... as above explain  */
    if(! (i%2 || i == 0) ) 
    {
        /* Record 2,4,6,8,10,12,14,16 for DATABIT, the data is MSB */  
        ch |= (status<<((i-1)/2)); 
    }

    /* We just handle the STARTBIT  and 8 bits DATABIT, skip the STOPBIT */
    if(i == 18 ) /* Stop bit clock arrive  */
    {
        /* Stop the TC counter */
        stop_tc(RXD_CLOCK_CHN); 

        /* Clear AT91_PIN_PB5 pin interrupt */
        at91_sys_read(AT91_PIOB+PIO_ISR);

        /* Enable AT91_PIN_PB5 pin interrupt */
        at91_sys_write(AT91_PIOB+PIO_IER, 1<<5);

        /* Check buffer is overflow or not */
        if ( CIRC_SPACE(rx_ring.head, rx_ring.tail, CIRC_BUF_SIZE) >= 1 )
        {
            rx_ring.buf[rx_ring.head] = ch;
            rx_ring.head = (rx_ring.head + 1) & (CIRC_BUF_SIZE - 1);
        }

        i = ch = 0;
        return IRQ_HANDLED;
    }

    i++;
    return IRQ_HANDLED;
}

/* When there is Data incoming, the RXD pin should get a falling edge interrupt,
 * then we will start the RXD Timer Counter to receive data
 * */
static irqreturn_t rxdpin_intterupt_handler(int irq,void *dev_id)
{
    /* AT91 PIO Interrupt can only capture both FALLING EDGE and RISING EDGE 
     * interrupt, but we should only capture the FALLING EDGE interrupt here*/
    if(HIGHLEVEL == at91_get_gpio_value(irq_to_gpio(irq)) )
        return IRQ_HANDLED;

    /* Disable AT91_PIN_PB5 interrupt */
    at91_sys_write(AT91_PIOB+PIO_IDR, 1<<5);

    /* Clear the interrupt status */
    at91_sys_read(AT91_PIOB+PIO_ISR);

    /* We use 2*Baudrate to sample the code, to fix the uncertain clock sample point */
    start_tc(RXD_CLOCK_CHN, 2*baudrate);  /* Max Baudrate support 9600bps */

    return IRQ_HANDLED;
}

static int gstty_open(struct inode *inode, struct file *file)
{
    int result = 0;
    int num = MINOR(inode->i_rdev);
    
    /* Set the TXD Pin to GPIO output mode  */
    at91_set_gpio_input(RXD_GPIO_PIN, ENPULLUP);

    /* Set the RXD pin to interrupt mode */
    at91_set_gpio_output(TXD_GPIO_PIN, HIGHLEVEL);
    at91_set_deglitch(RXD_GPIO_PIN, 1);

    /* Request the RXD GPIO pin interrupt, when there is data arrive in this pin, there should be
     * a START BIT, which is low level, and it should be high level when the line is free. So when
     * data arrive, there should be a falling edge interrupt happend.*/
    result = request_irq(RXD_GPIO_PIN, rxdpin_intterupt_handler, 0, DEV_NAME, (void *)num);
    if( result )
    {
        goto RET;
    }

    /* When the RXD GPIO pin get a falling edge interrupt, it means there is a data arrive, then it
     * will start the TC0 for RXD Receive Clock.
     */
    result = request_irq(RXD_CLOCK_IRQ, rxdtc_interrupt_handler, 0, DEV_NAME, (void *)num);
    if( result )
    {
        free_irq(RXD_GPIO_PIN, (void *)num);
        goto RET;
    }

    /* When the TXD GPIO pin get some data to send, it will use this Timer Counter to provide the 
     * data send out clock
     */
    result = request_irq(TXD_CLOCK_IRQ, txdtc_interrupt_handler, 0, DEV_NAME, (void *)num);
    if( result )
    {
        free_irq(RXD_GPIO_PIN, (void *)num);
        free_irq(RXD_CLOCK_IRQ, (void *)num);
        goto RET;
    }

    /* Always start SEND CLOCK TC channel */
    start_tc(TXD_CLOCK_CHN, baudrate); 

RET:
    return result;
}



static int gstty_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int len = 0;
    int size, left; 
    int to_end_space = 0;

    if(count<=0 || !access_ok(VERIFY_WRITE, buf, count) )
        return 0;

    /* In rxdtc_interrupt_handler() function, it will receive the data from RXD GPIO pin
     * and put the data into the Receive circle buffer. */
    if((size=CIRC_CNT(rx_ring.head, rx_ring.tail, CIRC_BUF_SIZE)) >= 1)
    { 
        left = len = size<=count? size : count; 

        /*  Circle buffer is not a continuous address space, this is tail to head left size */
        to_end_space = CIRC_CNT_TO_END(rx_ring.head, rx_ring.tail, CIRC_BUF_SIZE);

        if(left > to_end_space)
        {
            copy_to_user(buf, &rx_ring.buf[rx_ring.tail], to_end_space);
            rx_ring.tail = (rx_ring.tail + to_end_space) & (CIRC_BUF_SIZE - 1);
            left -= to_end_space;
        }
        else
        {
            to_end_space = 0;
        }

        copy_to_user(&buf[to_end_space], &rx_ring.buf[rx_ring.tail], left); 
        rx_ring.tail = (rx_ring.tail + left) & (CIRC_BUF_SIZE - 1);
    }

    if(size>count)
    {
        printk("WARNING: %s() buffer overflow: count=%d size=%d\n", __FUNCTION__, count, size);
    }
    return len;
}

static ssize_t gstty_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int len = 0;
    int size, left;
    int to_end_space = 0;

    DECLARE_WAITQUEUE(wait, current);

    if(count <= 0 || !access_ok(VERIFY_READ, buf, count) )
        return 0;

    /* If the send circle buffer is not full, put the send data into it. When the send circle buffer 
     * is not empty, the txdtc_interrupt_handler() will start to send the data out.*/
    if ( (size=CIRC_SPACE(tx_ring.head, tx_ring.tail, CIRC_BUF_SIZE)) >= 1 )
    {
        left = len = count<=size ? count : size;

        /*  Circle buffer is not a continuous address space, this is tail to head left size */
        to_end_space = CIRC_SPACE_TO_END(tx_ring.head, tx_ring.tail, CIRC_BUF_SIZE);

        if(left > to_end_space)
        {
            copy_from_user(&tx_ring.buf[tx_ring.head], buf, to_end_space);
            tx_ring.head = (tx_ring.head + to_end_space) & (CIRC_BUF_SIZE - 1);
            left -= to_end_space;
        }
        else
        {
             to_end_space = 0;
        }

        copy_from_user(&tx_ring.buf[tx_ring.head], &buf[to_end_space], left);
        tx_ring.head = (tx_ring.head + left) & (CIRC_BUF_SIZE - 1);
    }

    if(count>size)
    {
        printk("WARNING: %s() buffer overflow: count=%d size=%d\n", __FUNCTION__, count, size);
    }

    /* Add current process to wait queue, untill the data is send over by txdtc_interrupt_handler() */
    add_wait_queue(&tx_waitq, &wait);
    while(CIRC_CNT(tx_ring.head, tx_ring.tail, CIRC_BUF_SIZE) >=1)
    {
        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
        if (signal_pending(current))
        {
             len = -ERESTARTSYS;
             goto OUT;
        }
    }

OUT:
    remove_wait_queue(&tx_waitq, &wait);
    set_current_state(TASK_RUNNING);
    return len;
}

static long gstty_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int result = 0;

    return result;
}

static int gstty_release(struct inode *inode, struct file *file)
{
    int num = MINOR(inode->i_rdev);

    disable_irq(RXD_CLOCK_IRQ);
    free_irq(RXD_CLOCK_IRQ,(void *)num);

    disable_irq(TXD_CLOCK_IRQ);
    free_irq(TXD_CLOCK_IRQ,(void *)num);

    disable_irq(RXD_GPIO_PIN);
    free_irq(RXD_GPIO_PIN, (void *)num);

    return 0;
}


static struct file_operations gstty_fops = 
{ 
    .owner = THIS_MODULE,
    .open = gstty_open, 
    .read = gstty_read,
    .write = gstty_write,
    .unlocked_ioctl = gstty_ioctl,
    .release = gstty_release,
};

static int __init at91_gstty_init(void)
{
    int result = 0;
    dev_t  devno;

    /* Initialize the receive circle buffer */
    memset(&rx_ring, 0, sizeof(struct circ_buf));
    rx_ring.buf = kmalloc(CIRC_BUF_SIZE, GFP_KERNEL);
    if( NULL == rx_ring.buf )
        goto ERROR;

    /* Initialize the send circle buffer */
    memset(&tx_ring, 0, sizeof(struct circ_buf));
    tx_ring.buf = kmalloc(CIRC_BUF_SIZE, GFP_KERNEL);
    if( NULL == tx_ring.buf )
        goto ERROR;

    if( 0 != dev_major)
    {
        devno = MKDEV(dev_major, dev_minor);
        result = register_chrdev_region(devno, 1, DEV_NAME);
    }
    else
    {
        result = alloc_chrdev_region(&devno, dev_minor, 1, DEV_NAME);
        dev_major = MAJOR(devno);
    }

    /*  Alloc for device major failure */
    if (result < 0)
    {
        printk("%s driver can't get major %d\n", DEV_NAME, dev_major);
        return result;
    }

    dev_class = class_create(THIS_MODULE, DEV_NAME);
    if(IS_ERR(dev_class))
    {
        printk("%s driver create class failture\n",DEV_NAME);
        result =  -ENOMEM;
        goto ERROR;
    }

    cdev_init (&cdev, &gstty_fops); 
    cdev.owner  = THIS_MODULE; 
    result = cdev_add (&cdev, devno, 1); 
    if (result) 
    { 
        printk (KERN_NOTICE "error %d add %s device", result, DEV_NAME); 
        goto ERROR; 
    }


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)     
    device_create(dev_class, NULL, devno, NULL, "%s%d", DEV_NAME, 0); 
#else 
    device_create(dev_class, NULL, devno, "%s%d", DEV_NAME, 0);
#endif

    /* Alloc for the Timer Counter from drivers/misc/atmel_tclib.c, it depends on 
     * the kernel configure for ATMEL TC library support in drivers->misc */
    tc = atmel_tc_alloc(AT91_TCBLOCK0, "recv_tc");
    if (!tc) 
    { 
        printk("can't alloc TC for clocksource\n"); 
        result = -ENODEV;
        goto ERROR;
    }
    tcaddr = tc->regs;

    printk("AT91 %s driver version %d.%d.%d <%s> initiliazed.\n", 
            DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER, __DATE__);
    return 0;

ERROR:

    if(tx_ring.buf != NULL)
        kfree(tx_ring.buf);

    if(rx_ring.buf != NULL)
        kfree(rx_ring.buf);

    printk("AT91 %s driver version %d.%d.%d <%s> install failure.\n", 
            DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER, __DATE__);
    cdev_del(&cdev);
    unregister_chrdev_region(devno, 1);

    return result;
}

static void at91_gstty_exit(void)
{
    atmel_tc_free(tc);

    device_destroy(dev_class, MKDEV(dev_major, 0));

    class_destroy(dev_class);
    cdev_del(&cdev);

    unregister_chrdev_region(MKDEV(dev_major, 0), 1);
    printk("AT91 %s driver removed\n", DEV_NAME);
    return;
}

module_init(at91_gstty_init);
module_exit(at91_gstty_exit);

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:AT91SAM9260_GSTTY");

