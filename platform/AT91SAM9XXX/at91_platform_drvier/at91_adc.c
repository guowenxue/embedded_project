/*********************************************************************************
 *      Copyright:  (C) 2011 Guo Wenxue<guowenxue@gmail.com>  
 *                  All rights reserved.
 *
 *       Filename:  at91_adc.c
 *    Description:  This file is the Analog-to-digital Converter(ADC) Driver on AT91.
 *                 
 *        Version:  1.0.0(11/17/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "11/17/2011 04:40:39 PM"
 *
 *      Reference:  http://www.at91.com/forum/viewtopic.php/f,12/t,4992/
 *                  http://www.hackchina.com/r/75269/adc.h__html
 *                  
 *                 
 ********************************************************************************/

#include "at91_driver.h"

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9XXX  Analog-to-digital Converter(ADC) driver"

/*  Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_ADC_NAME
//#define DEV_MAJOR                 DEV_ADC_MAJOR

#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*   dynamic major by default */ 
#endif

#define ADC_CHN0                  0  /* This Pin is for SIM/SAM SCK3 on MiniSTAC  */
#define ADC_CHN1                  1  /* This Pin is for restore key on MiNiSTAC */
#define ADC_CHN2                  2  /* This Pin is for Beep on MiniSTAC  */
#define ADC_CHN3                  3  /* This Pin is for LED0 on MiniSTAC  */
#define MAX_ADC_CHN               4
#define ADC_CHN_CNT               2

#define DATA_LEN                  10   /* MAX save 10 sample datas */
#define STD_VOLTAGE               3300 /* 3.3V=3300mv */
#define PRECISION                 1024 /* We use 10bit ADC */

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;
static unsigned long sample_interval=100;  /* 1second=100jiffies */

static struct resource at91_adc_resources[] = {
    [0] = {
        .start = AT91SAM9260_BASE_ADC,
        .end   = AT91SAM9260_BASE_ADC + SZ_16K - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = AT91SAM9260_ID_ADC,
        .end   = AT91SAM9260_ID_ADC,
        .flags = IORESOURCE_IRQ,
    },
};

unsigned int adc_channel_gpio[MAX_ADC_CHN] = {
    AT91_PIN_PC0, AT91_PIN_PC1, AT91_PIN_PC2, AT91_PIN_PC3
};

/* AT91SAM9260 MAX support 4 ADC channels from PC0~PC3  */
struct adc_channel
{
    int                 id;         /* Which port we used, AT91_PIN_PC0~AT91_PIN_PC3 */
    unsigned int        sample_data[DATA_LEN];
    struct timer_list   sample_timer;
    struct cdev         cdev; 
    int                 devno;
};

struct adc_dev
{
    void __iomem        *io;
    struct clk          *clk;
    struct class        *class;
    int                 chn_cnt;
    struct adc_channel  *channel;
    spinlock_t          lock;
};
static struct adc_dev *adc;


static void platform_adc_release(struct device * dev)
{
    return ;
}

static struct platform_device at91_adc_device = { 
    .name      = "at91_adc",
    .id        = -1,
    .resource   = at91_adc_resources,
    .num_resources  = ARRAY_SIZE(at91_adc_resources),
    .dev = {
        .release = platform_adc_release,
    }
};

void adc_sample_timer_handler(unsigned long index)
{
    struct adc_channel  *channel = &(adc->channel[index]);
    int i;

#if 0
    { 
        struct timeval time;
        do_gettimeofday(&time);
        printk(KERN_INFO "sample_interval:%lu Time %ld:%ld\n",sample_interval, time.tv_sec, time.tv_usec);
    }
#endif

    spin_lock(&adc->lock);
    __raw_writel(AT91_ADC_START, (adc->io + AT91_ADC_CR)); /*  Trigger the ADC  */ 
    spin_unlock(&adc->lock);

#if 0
    printk("ADC Channel[%d]   ADC_MR register: 0x%x.\n", channel->id,  ioread32(adc->io + AT91_ADC_MR));
    printk("ADC Channel[%d] ADC_CHSR register: 0x%x.\n", channel->id,  ioread32(adc->io + AT91_ADC_CHSR));
    printk("ADC Channel[%d]   ADC_SR register: 0x%x.\n", channel->id,  ioread32(adc->io + AT91_ADC_SR));
    printk("ADC Channel[%d]  ADC_IMR register: 0x%x.\n", channel->id,  ioread32(adc->io + AT91_ADC_IMR));
#endif

    /* Wait for conversion to be complete. */
    while ((__raw_readl(adc->io+AT91_ADC_SR) & (1<<channel->id)) == 0) 
    {
        //printk("ADC Channel[%d]  ADC_SR register: 0x%x.\n", channel->id,  ioread32(adc->io+AT91_ADC_SR));
        cpu_relax();
    }

    for(i=DATA_LEN-1; i>0; i--)
    {
        channel->sample_data[i]=channel->sample_data[i-1];
    }
    /* Calculate current voltage, unit mv */
    channel->sample_data[0] = STD_VOLTAGE*ioread32(adc->io + AT91_ADC_CHR(channel->id))/PRECISION;

#if 0
    printk("ADC Channel[%d] sample new data: %u.\n", channel->id,  channel->sample_data[0]);
    for(i=0; i<DATA_LEN; i++)
        printk("%u ", channel->sample_data[i]);
    printk("\n");
#endif

    spin_lock(&adc->lock);
    mod_timer(&(channel->sample_timer), jiffies+sample_interval);
    spin_unlock(&adc->lock);
}

static int adc_open(struct inode *inode, struct file *file)
{ 
    int  index; 
    index = iminor(file->f_path.dentry->d_inode); /* Which Channel*/ 

    file->private_data = &(adc->channel[index]);
    return 0;
}

static int adc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct adc_channel *channel = file->private_data;
    int len = 0;

    len = min(sizeof(channel->sample_data), count);

#if 0
    { 
        int i; 
        printk("Read ADC sample %d bytes data:\n", len); 
        for(i=0; i<DATA_LEN; i++) 
        { 
            printk("%d ", channel->sample_data[i]); 
        } 
        printk("\n");
    }
#endif
    copy_to_user(buf, channel->sample_data, len);

    return len;
}

static int adc_release(struct inode *inode, struct file *file)
{
    return 0;
}

static void print_help(void)
{ 
    printk("Follow is the ioctl() command for %s driver:\n", DEV_NAME); 
    printk("Enable Driver debug command     : %u\n", SET_DRV_DEBUG); 
    printk("Get Driver verion  command      : %u\n", GET_DRV_VER); 
    printk("Set ADC sample interval command : %u\n", ADC_SET_INTERVEL); 
}


static long adc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) 
    { 
        case SET_DRV_DEBUG: 
            dbg_print("%s driver debug now.\n", DISABLE == arg ? "Disable" : "Enable"); 
            debug = (0==arg) ? DISABLE : ENABLE; 
            break; 

        case GET_DRV_VER: 
            print_version(DRV_VERSION); 
            return DRV_VERSION;

        case ADC_SET_INTERVEL: 
            spin_lock(&adc->lock); 
            sample_interval = arg;
            spin_unlock(&adc->lock);
            dbg_print("Set sample_interval time: %lu\n", sample_interval);
            break;

        default:
             dbg_print("%s driver don't support ioctl command=%d\n", DEV_NAME, cmd);
             print_help();
             return -EINVAL;
    }

    return 0;
}


static struct file_operations adc_fops = { 
    .owner = THIS_MODULE, 
    .open = adc_open, 
    .read = adc_read, 
    .release = adc_release, 
    .unlocked_ioctl = adc_ioctl, /*  compatible with kernel version >=2.6.38*/
};

/* Malloc for the adc device and channels  */
struct adc_dev * setup_adc_device(void)
{  
    int i,ret = 0;
    struct adc_dev *adc = NULL;
    struct adc_channel *channel = NULL;

    adc = kmalloc(sizeof(struct adc_dev), GFP_KERNEL);
    if (!adc) 
        goto ERR_ADC_MALLOC;

    memset(adc, 0, sizeof(struct adc_dev));

    adc->class = class_create(THIS_MODULE, DEV_NAME);
    if(IS_ERR(adc->class)) 
    { 
        printk("%s driver create class failture\n",DEV_NAME);
        goto ERR_CLASS_CREAT;
    }

    adc->chn_cnt = 2;
    adc->channel = kmalloc(adc->chn_cnt*sizeof(struct adc_channel), GFP_KERNEL);
    if(!adc->channel)
    {
        goto ERR_CHANNEL_MALLOC;
    }

    /* We used ADC channel 1,3  */
    adc->channel[0].id = ADC_CHN1; 
    adc->channel[1].id = ADC_CHN3; 

    for(i=0; i<adc->chn_cnt; i++)
    {
        channel = &(adc->channel[i]);

        memset(channel->sample_data, 0, sizeof(channel->sample_data));

        /* Setup the character device  */
        channel->devno = MKDEV(dev_major, dev_minor+i);
        cdev_init(&channel->cdev, &adc_fops);
        channel->cdev.owner = THIS_MODULE; 
        channel->cdev.ops = &adc_fops;

        /* Every ADC channel create a character device */
        ret = cdev_add (&channel->cdev, channel->devno, 1);
        if(ret)
        {
            printk(KERN_NOTICE "Error[%d] adding %s%d", ret, DEV_NAME, i);
            goto ERR_DEV_CREATE;
        }
        else
        { 
            device_create(adc->class, NULL, channel->devno, NULL, "%s%d", DEV_NAME, i);
            dbg_print("cdev_add %s%d successfully.\n", DEV_NAME, i);
        }
    }

    dbg_print("setup_adc_device() ok.\n");

    return adc;

ERR_DEV_CREATE:
    kfree(adc->channel);

ERR_CHANNEL_MALLOC:
    class_destroy(adc->class);

ERR_CLASS_CREAT:
    kfree(adc);

ERR_ADC_MALLOC:
    return NULL;
}

static void initialize_adc_channel( struct adc_dev *adc )
{
    int i;
    struct adc_channel *channel = NULL;

    adc->clk=clk_get(NULL,"adc_clk"); 
    clk_enable(adc->clk);

    dbg_print("%s[%d] %s()\n", __FILE__,__LINE__,__FUNCTION__);

    /* Reset the the ADC  */
    __raw_writel(AT91_ADC_SWRST, (adc->io + AT91_ADC_CR));

    /* Disable all ADC interrupter, we use software timer to sample */
    __raw_writel(0x0F0F0F, (adc->io+AT91_ADC_IDR) );

    /* Configure ADC mode register:
     *  From 39.7.2 ADC Mode Register P715~P716 of AT91SAM9260 user manual: 
     *  Maximum ADC clock frequency = 5MHz = MCK / ((PRESCAL+1) * 2) 
     *     PRESCAL = ((MCK / 5MHz) / 2) -1 = ((100MHz / 5MHz)/2)-1) = 9 
     *  Maximum startup time = 15uS = (STARTUP+1)*8/ADC_CLOCK 
     *     STARTUP = ((15uS*ADC_CLOK)/8)-1 = ((15uS*5MHz)/8)-1 = 9 
     *  Minimum hold time = 1.2uS = (SHTIM+1)/ADC_CLOCK 
     *  SHTIM = (1.2uS*ADC_CLOCK)-1 = (1.2uS*5MHz)-1 = 5, Use 9 to ensure 2uS hold time.  
     *  Enable sleep mode and disable hardware trigger from TIOA output from TC0.we use 
     *  software timer.
     */
    __raw_writel((AT91_ADC_SHTIM_(9) | AT91_ADC_STARTUP_(9) | AT91_ADC_PRESCAL_(9) | 
                AT91_ADC_SLEEP ), (adc->io + AT91_ADC_MR));

    for(i=0; i<adc->chn_cnt; i++)
    { 
        channel = &(adc->channel[i]);

        /* Set the GPIO port as ADC functional */ 
        at91_set_A_periph(adc_channel_gpio[channel->id], 0);

        /* Enable all the ADC channels */
        __raw_writel(AT91_ADC_CH(channel->id), (adc->io+AT91_ADC_CHER));

        /* Add the ADC sample timer */
        init_timer(&(channel->sample_timer));
        channel->sample_timer.function = adc_sample_timer_handler;
        channel->sample_timer.data = (unsigned long)i;
        channel->sample_timer.expires  = jiffies + sample_interval;
        add_timer(&(channel->sample_timer));
    }

    return ;
}

static int at91_adc_remove(struct platform_device *pdev)
{
    struct adc_dev *adc = platform_get_drvdata(pdev);
    struct resource *res;
    int size = 0;

    iounmap(adc->io);

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if( !res )
    {
        printk("%s[%04d] failed to get %s memory regist.\n", __FILE__, __LINE__, DEV_NAME);
    }
    else
    { 
        size = resource_size(res); 
        release_mem_region(res->start, size);
    }

    if(NULL == adc)
    {
        goto RET;
    }

    if(NULL == adc->channel) 
    {
        kfree(adc);
        goto RET;
    }

    if(NULL != adc->channel) 
    { 
        int i;
        for(i=0; i<adc->chn_cnt; i++)
        {
            dbg_print("Desdroy %s%d ID=%d.\n", DEV_NAME, i, adc->channel[i].id);
            cdev_del(&(adc->channel[i].cdev));
            device_destroy(adc->class, adc->channel[i].devno);
            del_timer(&(adc->channel[i].sample_timer));
        } 
    }

    class_destroy(adc->class);
  
    kfree(adc->channel); 
    kfree(adc);

    unregister_chrdev_region(MKDEV(dev_major, dev_minor), 1);
    printk("AT91 %s driver removed\n", DEV_NAME);

RET:
    platform_set_drvdata(pdev, NULL);
    return 0;
}


static int at91_adc_probe(struct platform_device *pdev)
{
    struct resource *res;
    int size;
    int ret = 0;
    dev_t devno;

    /* Alloc the device for driver */ 
    if (0 != dev_major) 
    { 
        devno = MKDEV(dev_major, dev_minor); 
        ret = register_chrdev_region(devno, 1, DEV_NAME); 
    } 
    else 
    { 
        ret = alloc_chrdev_region(&devno, dev_minor, 1, DEV_NAME); 
        dev_major = MAJOR(devno); 
    } 
    
    if (ret < 0) 
    {    
        printk(KERN_WARNING "%s: can't get major %d\n", DEV_NAME, dev_major); 
        return ret;   
    }  

    /* Set up the device information and add the device */
    adc = setup_adc_device();
    if(NULL == adc)
    {
        printk("setup_adc_device() failure: ret=%d\n", ret);
        return ret;
        goto adc_del_dev;
    }
    spin_lock_init(&adc->lock);

    platform_set_drvdata(pdev, adc);

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if( !res )
    {
        printk("%s[%04d] failed to get %s memory regist.\n", __FILE__, __LINE__, DEV_NAME);
        ret = -ENXIO;
        goto adc_del_channel;
    }

    size = resource_size(res); 
    if( !request_mem_region(res->start, size, pdev->name) )
    {
        printk("%s[%04d] failed to get %s memory region.\n", __FILE__, __LINE__, DEV_NAME);
        ret = -ENOENT;
        goto adc_del_channel;
    }

    adc->io = ioremap(res->start, size);
    if( !adc->io )
    {
        printk("%s[%04d] %s ioremap() of registers failed.\n", __FILE__, __LINE__, DEV_NAME);
        ret = -ENXIO;
        goto adc_release_mem;
    }

    initialize_adc_channel(adc);

    printk("at91_adc driver probe successfully!\n");
    return 0;

adc_release_mem:
    release_mem_region(res->start, size);

adc_del_channel:
    at91_adc_remove(pdev);

adc_del_dev: 
    unregister_chrdev_region(MKDEV(dev_major, dev_minor), 1);

    return ret;
}



static struct platform_driver at91_adc_driver = { 
    .probe      = at91_adc_probe, 
    .remove     = at91_adc_remove, 
    .driver     = { 
        .name       = "at91_adc", 
        .owner      = THIS_MODULE, 
    },
};


static int __init at91_adc_init(void)
{
    int       ret = 0;

    ret = platform_device_register(&at91_adc_device);
    if(ret)
    {
        printk(KERN_ERR "%s:%d: Can't register platform device %s: %d\n", __FUNCTION__,__LINE__,DEV_NAME,ret);
        goto fail_reg_plat_dev;
    }
    dbg_print("Regist AT91 platform %s device successfully.\n", DEV_NAME);

    ret = platform_driver_register(&at91_adc_driver);
    if(ret)
    {
        printk(KERN_ERR "%s:%d: Can't register platform driver %s: %d\n", __FUNCTION__,__LINE__,DEV_NAME,ret);
        goto fail_reg_plat_drv;
    }
    dbg_print("Regist AT91 platform %s driver successfully.\n", DEV_NAME);

    return 0;

fail_reg_plat_drv:
    platform_driver_unregister(&at91_adc_driver);

fail_reg_plat_dev:
    return ret;
}

static void at91_adc_exit(void)
{ 
    dbg_print("%s():%d remove %s platform drvier\n", __FUNCTION__,__LINE__,DEV_NAME); 
    platform_driver_unregister(&at91_adc_driver); 

    dbg_print("%s():%d remove %s platform device\n", __FUNCTION__,__LINE__,DEV_NAME); 
    platform_device_unregister(&at91_adc_device);
}


module_init(at91_adc_init);
module_exit(at91_adc_exit);

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:AT91SAM9260_ADC");

