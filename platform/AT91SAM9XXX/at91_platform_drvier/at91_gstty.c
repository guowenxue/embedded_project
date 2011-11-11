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
 ********************************************************************************/

#include "at91_driver.h"

#include <mach/at91_tc.h>
#include <linux/atmel_tc.h>
#include <mach/at91_pio.h>
#include <linux/clk.h>

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


static void __iomem    *tcaddr;
struct atmel_tc        *tc; 

static struct cdev cdev;
static struct class *dev_class = NULL;

#define AT91_TCBLOCK0             0 /* Time Counter Block 0, for TC0,TC1,TC2  */
#define AT91_TCBLOCK1             1 /* Time Counter Block 1, for TC3,TC4,TC5  */

#define AT91_TC_CHN0              0 /* Channel 0 in a TC Block */
#define AT91_TC_CHN1              1 /* Channel 1 in a TC Block */
#define AT91_TC_CHN2              2 /* Channel 2 in a TC Block */

#define CIRC_BUF_SIZE             1024

struct gsuart
{
    unsigned int           rxd_gpio;    /* Which PIN used as Receive  */
    unsigned int           txd_gpio;    /* Which PIN used as Transmit */
};

struct circ_buf  rx_ring;
struct circ_buf  tx_ring;

static struct gsuart  gsuarts[] = {
    [0] = {
        .txd_gpio = AT91_PIN_PB4,
        .rxd_gpio = AT91_PIN_PB5,
    },
};
#define GSUART_NUMS      ARRAY_SIZE(gsuarts)

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

    //printk("mck_rat_hz=%lu RC=%lu\n", mck_rate_hz, mck_rate_hz/(2*baudrate));
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

    clear_tc(AT91_TC_CHN0);

    status = (at91_sys_read(AT91_PIOB+PIO_PDSR)>>5)&0x01;

    /* Skip sample Clock0, clock1, clock3, clock5... as above explain  */
    if(! (i%2 || i == 0) ) 
        ch |= (status<<((i-1)/2));  

    if(i == 18 ) /* Stop bit clock arrive  */
    {
        /* Stop the TC counter */
        stop_tc(AT91_TC_CHN0); 

        /* Clear AT91_PIN_PB5 pin interrupt */
        at91_sys_read(AT91_PIOB+PIO_ISR);

        /* Enable AT91_PIN_PB5 pin interrupt */
        at91_sys_write(AT91_PIOB+PIO_IER, 1<<5);

        /* Check buffer is overflow or not */
        if ( CIRC_SPACE(rx_ring.head, rx_ring.tail, CIRC_BUF_SIZE) >= 1 )
        {
            //printk("rx_ring.head=%d\n", rx_ring.head);
            rx_ring.buf[rx_ring.head] = ch;
            rx_ring.head = (rx_ring.head + 1) & (CIRC_BUF_SIZE - 1);
        }
#if 0
        else /* Buffer overflow  */
        {
            int j=0;
            for(j=0; j<rx_ring.head; j++)
            {
                printk("buf[%d]=0x%x\n", j, rx_ring.buf[j]);
            }
            printk("Head=%d\n", rx_ring.head);
            memset(rx_ring.buf, 0xa5, CIRC_BUF_SIZE);
            rx_ring.head = rx_ring.tail = 0;
        }
#endif
        i = ch = 0;
        return IRQ_HANDLED;
    }

    i++;
    return IRQ_HANDLED;
}

/* When there is Data incoming, the RXD pin should get a falling edge interrupt */
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
    start_tc(AT91_TC_CHN0, 2*9600);  /* Max Baudrate support 9600bps */

    return IRQ_HANDLED;
}

static int gstty_open(struct inode *inode, struct file *file)
{
    int result = 0;
    int num = MINOR(inode->i_rdev);
    struct gsuart *pgsuart = NULL;
    
    pgsuart = file->private_data = &gsuarts[num];

    /* Set the RXD pin to interrupt mode */
    at91_set_gpio_input(pgsuart->rxd_gpio, ENPULLUP);
    at91_set_deglitch(pgsuart->rxd_gpio, 1);
    result = request_irq(pgsuart->rxd_gpio, rxdpin_intterupt_handler, 0, DEV_NAME, (void *)num);
    if( result )
    {
         result = -EBUSY; 
         return -EBUSY;
    }

    result = request_irq(AT91SAM9260_ID_TC0, rxdtc_interrupt_handler, 0, DEV_NAME, (void *)num);
    if( result )
    {
         result = -EBUSY; 
         return -EBUSY;
    }

    return result;
}



static int gstty_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int len = 0;
    int size = 0;

    if((size=CIRC_CNT(rx_ring.head, rx_ring.tail, CIRC_BUF_SIZE)) >= 1)
    {
       len = size>count? count : size; 
       copy_to_user(buf, &rx_ring.buf[rx_ring.tail], len);
       rx_ring.tail = (rx_ring.tail + len) & (CIRC_BUF_SIZE - 1); 
    }
    else
        return 0;

    return len;
}

static ssize_t gstty_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int result = 0;

    return result;
}

static long gstty_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int result = 0;

    return result;
}

static int gstty_release(struct inode *inode, struct file *file)
{
    int result = 0;
    int num = MINOR(inode->i_rdev);
    struct gsuart *pgsuart = NULL;

    pgsuart = file->private_data;

    disable_irq(AT91SAM9260_ID_TC0);
    free_irq(AT91SAM9260_ID_TC0,(void *)num);

    disable_irq(pgsuart->rxd_gpio);
    free_irq(pgsuart->rxd_gpio, (void *)num);

    return result;
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

    memset(&rx_ring, 0, sizeof(struct circ_buf));
    rx_ring.buf = kmalloc(CIRC_BUF_SIZE, GFP_KERNEL);
    if( NULL == rx_ring.buf )
        goto ERROR;

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

    tc = atmel_tc_alloc(AT91_TCBLOCK0, "recv_tc");
    if (!tc) 
    { 
        printk("can't alloc TC for clocksource\n"); 
        result = -ENODEV;
        goto ERROR;
    }
    tcaddr = tc->regs;

    printk("AT91 %s driver version %d.%d.%d initiliazed.\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER);
    return 0;

ERROR:

    if(tx_ring.buf != NULL)
        kfree(tx_ring.buf);

    if(rx_ring.buf != NULL)
        kfree(rx_ring.buf);

    printk("AT91 %s driver version %d.%d.%d install failure.\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER);
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

