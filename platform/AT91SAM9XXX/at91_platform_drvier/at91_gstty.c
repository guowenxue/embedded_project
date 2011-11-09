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
#define ARCH_ID_AT91SAM9260       0x019803a0
#define ARCH_ID_AT91SAM9G20       0x019905a0

struct gsuart
{
    unsigned int           rxd_gpio;    /* Which PIN used as Receive  */
    unsigned int           txd_gpio;    /* Which PIN used as Transmit */
};

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

    /* Get the MCK */
    mck_rate_hz = clk_get_rate(tc->clk[channel]);

    /* Clock Disable */
    __raw_writel(AT91_TC_CLKDIS, tcaddr + ATMEL_TC_REG(channel, CCR));  

    /* All interrupt disable */
    __raw_writel(0xFF, tcaddr + ATMEL_TC_REG(channel, IDR));  

    /* Choose External Clock1(MCK/2),RC Compare triger */ 
    __raw_writel(AT91_TC_TIMER_CLOCK1| AT91_TC_CPCTRG, tcaddr + ATMEL_TC_REG(channel, CMR));  

    /* Set the counter value */
    __raw_writel((mck_rate_hz/(2*baudrate)), tcaddr + ATMEL_TC_REG(channel, RC));  

    /* RC compare interrupt enable */ 
    __raw_writel((1<<4), tcaddr + ATMEL_TC_REG(channel, IER));  

    /* Clear the TC status  */
    __raw_readl(tcaddr + ATMEL_TC_REG(channel, SR));

    /* Enable the TC Clock in AT91_PMC_PCER(Can Read status from AT91_PMC_PCSR) */
    clk_enable(tc->clk[channel]);

    /* software trigger and clock enable */
    __raw_writel((AT91_TC_SWTRG | AT91_TC_CLKEN), tcaddr + ATMEL_TC_REG(channel, CCR));  
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

#define LEN 8 
static unsigned char data[LEN] = {0}; 

/* This is the Receive Time Counter interrupt handler, which used to provide receive clock  */
static irqreturn_t gstty_rxdtc_interrupt(int irq, void *dev_id)
{
    static int count=0;
    clear_tc(AT91_TC_CHN0);

    /* Get AT91_PIN_PB5 Pin value */
    data[count] = (at91_sys_read(AT91_PIOB+PIO_PDSR)>>5)&0x01;
    //data[count] = at91_get_gpio_value(irq_to_gpio(irq));

    if(count==LEN)
    {
        int i;
        stop_tc(AT91_TC_CHN0);
        printk("count=%d\n", count);
        for(i=0; i<LEN; i++)
            printk(" data[%d]=%d\n", i, data[i]);
//        memset(data, 0, LEN);
        count = 0;

        /* Enable AT91_PIN_PB5 pin interrupt */
        at91_sys_write(AT91_PIOB+PIO_IER, 1<<5);
    }

    count++;
    return IRQ_HANDLED;
}

/* This is the RXD GPIO Pin interrupt handler */
static irqreturn_t recv_intterupt_handler(int irq,void *de_id)
{
    /* The start bit must be lowlevel */
    if(HIGHLEVEL == at91_get_gpio_value(irq_to_gpio(irq)) )
        return IRQ_HANDLED;

    /* Disable AT91_PIN_PB5 pin interrupt */
    at91_sys_write(AT91_PIOB+PIO_IDR, 1<<5);

    start_tc(AT91_TC_CHN0, 1200);

    return IRQ_HANDLED;
}

static int gstty_open(struct inode *inode, struct file *file)
{
    int result = 0;
    int num = MINOR(inode->i_rdev);
    struct gsuart *pgsuart = NULL;
    
    pgsuart = file->private_data = &gsuarts[num];
    printk("num=%d\n", num);

    /* Set the RXD pin to interrupt mode */
    at91_set_gpio_input(pgsuart->rxd_gpio, ENPULLUP);
    at91_set_deglitch(pgsuart->rxd_gpio, 1);
    result = request_irq(pgsuart->rxd_gpio, recv_intterupt_handler, 0, DEV_NAME, (void *)num);
    if( result )
    {
         result = -EBUSY; 
         return -EBUSY;
    }

    result = request_irq(AT91SAM9260_ID_TC0, gstty_rxdtc_interrupt, 0, DEV_NAME, (void *)num);
    if( result )
    {
         result = -EBUSY; 
         return -EBUSY;
    }

    return result;
}



static int gstty_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int result = 0;



    return result;
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


#if 0
    request_mem_region(AT91SAM9260_BASE_TCB0, 0x4000, "recv_tc");
    tcaddr = ioremap(AT91SAM9260_BASE_TCB0, 0x4000);
#endif


    printk("AT91 %s driver version %d.%d.%d initiliazed.\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER);
    return 0;

ERROR:
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

