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
static unsigned long mck_rate_hz;
static unsigned long main_clk_rate_hz;
static unsigned long plla_rate_hz;
static unsigned long cpu_clk_rate_hz;

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

static inline int at91_get_clock(unsigned long main_clock)
{ 
    unsigned regs, freq; 
    main_clk_rate_hz = main_clock;

    regs = at91_sys_read(AT91_CKGR_PLLAR); 
    plla_rate_hz = at91_pll_rate(main_clock, regs);

    regs = at91_sys_read(AT91_PMC_MCKR);
    mck_rate_hz = at91_css_to_rate(regs & AT91_PMC_CSS);
    freq = mck_rate_hz;
    freq /= (1 << ((regs & AT91_PMC_PRES) >> 2)); /*  prescale */ 

    if(ARCH_ID_AT91SAM9G20 == (at91_sys_read(AT91_DBGU_CIDR) & ~AT91_CIDR_VERSION))
    {
        /*  mdiv ; (x >> 7) = ((x >> 8) * 2) */
        mck_rate_hz = (regs & AT91_PMC_MDIV) ?  freq / ((regs & AT91_PMC_MDIV) >> 7) : freq;
        if (regs & AT91_PMC_MDIV)
            freq /= 2;          /*  processor clock division */
    }
    else if(ARCH_ID_AT91SAM9260 == (at91_sys_read(AT91_DBGU_CIDR) & ~AT91_CIDR_VERSION))
    {
        mck_rate_hz = freq / (1 << ((regs & AT91_PMC_MDIV) >> 8));
    }

    cpu_clk_rate_hz = freq;

    printk("Clocks: CPU %lu MHz, master %u MHz, main %u.%03u MHz\n",
            cpu_clk_rate_hz / 1000000, (unsigned) mck_rate_hz / 1000000,
            (unsigned) main_clk_rate_hz / 1000000, ((unsigned) main_clk_rate_hz % 1000000) / 1000);

    return 0;
}


/* Start the TC channel to provide the receive/send data clock */
static inline void start_tc(int channel, unsigned long baudrate)
{
    /* Clock Disable */
    __raw_writel(AT91_TC_CLKDIS, tcaddr + ATMEL_TC_REG(channel, CCR));  

    /* All interrupt disable */
    __raw_writel(0xFF, tcaddr + ATMEL_TC_REG(channel, IDR));  

    /* Choose External Clock1(MCK/2),RC Compare triger */ 
    __raw_writel(AT91_TC_TIMER_CLOCK1| AT91_TC_CPCTRG, tcaddr + ATMEL_TC_REG(channel, CMR));  

    /* Set the counter value */
    __raw_writel(65535-(mck_rate_hz/(2*baudrate)), tcaddr + ATMEL_TC_REG(channel, RC));  

    /* RC compare interrupt enable */ 
    __raw_writel((1<<4), tcaddr + ATMEL_TC_REG(channel, IER));  

    /* software trigger and clock enable */
    __raw_writel((AT91_TC_SWTRG | AT91_TC_CLKEN), tcaddr + ATMEL_TC_REG(channel, CCR));  
}

/* Stop the TC channel  */
static void stop_tc(int channel)
{
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

#define LEN 16
static unsigned char data[LEN] = {0}; 

/* This is the Receive Time Counter interrupt handler, which used to provide receive clock  */
static irqreturn_t gstty_rxdtc_interrupt(int irq, void *dev_id)
{
    static int count = 0;
    count++;

    if(!(count%1000))
    {
        printk("ms=%d count=%d\n",jiffies_to_msecs(jiffies), count); 
        return IRQ_HANDLED;
    }
    else
        return IRQ_HANDLED;


    /* Get AT91_PIN_PB5 Pin value */
    data[count] = (at91_sys_read(AT91_PIOB+PIO_PDSR)>>5)&0x01;

    if(count>=LEN)
    {
        int i;
        stop_tc(AT91_TC_CHN0);
        printk("count=%d\n", count);
        for(i=0; i<LEN; i++)
            printk(" data[%d]=%d\n", i, data[i]);
        memset(data, 0, LEN);
        count = 0;

        /* Enable AT91_PIN_PB5 pin interrupt */
        at91_sys_write(AT91_PIOB+PIO_IER, 1<<5);
    }

    count++;
    return IRQ_HANDLED;
}

static struct irqaction gstty_rxdtc_irq = 
{ 
    .name       = "at91_tick", 
    .flags      = IRQF_DISABLED | IRQF_TIMER, 
    .handler    = gstty_rxdtc_interrupt,
};

/* This is the RXD GPIO Pin interrupt handler */
static irqreturn_t recv_intterupt_handler(int irq,void *de_id)
{
    /* The start bit must be lowlevel */
    if(HIGHLEVEL == at91_get_gpio_value(irq_to_gpio(irq)) )
        return IRQ_HANDLED;

    /* Disable AT91_PIN_PB5 pin interrupt */
    at91_sys_write(AT91_PIOB+PIO_IDR, 1<<5);

    //start_tc(AT91_TC_CHN0, 115200);
    //start_tc(AT91_TC_CHN0, 115200);
    start_tc(AT91_TC_CHN0, 1000);

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

    setup_irq(AT91SAM9260_ID_TC0, &gstty_rxdtc_irq);

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

    remove_irq(AT91SAM9260_ID_TC0, &gstty_rxdtc_irq);

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

    at91_get_clock(18432000); /* 18.432 MHz crystal, used to get mck_rate_hz*/ 

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
