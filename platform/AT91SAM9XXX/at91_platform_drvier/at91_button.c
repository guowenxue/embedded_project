/*********************************************************************************
 *      Copyright:  (C) 2011 Guo Wenxue<guowenxue@gmail.com>  
 *                  All rights reserved.
 *
 *       Filename:  at91_button.c
 *    Description:  This is the common button driver runs on AT91SAM9260/SAM9G20.
 *                 
 *        Version:  1.0.0(10/27/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "10/27/2011 11:39:10 AM"
 *                 
 ********************************************************************************/
#include "at91_driver.h"

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9XXX button driver"

/* Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_BUTTON_NAME

//#define DEV_MAJOR               DEV_BUTTON_MAJOR
#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /* dynamic major by default */
#endif

#define BUTTON_UP                 0 /* Button status is up */
#define BUTTON_DOWN               1 /* Button status is pushed down */
#define BUTTON_UNCERTAIN          2 /* Button status uncerntain */

#define TIMER_DELAY_DOWN          (HZ/50)   /*Remove button push down dithering timer delay 20ms  */
#define TIMER_DELAY_UP            (HZ/10)   /*Remove button up dithering timer delay 100ms  */

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;


/*============================ Platform Device part ===============================*/
/* Button hardware informtation structure*/
struct at91_button_info
{
    unsigned char           num;       /*Button nubmer  */
    char *                  name;      /*Button nubmer  */
    int                     nIRQ;      /*Button IRQ number*/
    unsigned int            irq_type;  /*Button IRQ flags */
    unsigned int            gpio;      /*Button GPIO port */
};

/* The button plaotform device private data structure */
struct at91_button_platform_data
{
    struct at91_button_info *buttons;
    int                     nbuttons;
};

/* Button hardware informtation data*/
static struct at91_button_info  at91_buttons[] = {
    [0] = {
        .num = 1,
        .name = "restore",
        .nIRQ = AT91_PIN_PB20,
        .gpio = AT91_PIN_PB20,
        /* AT91SAM9260 ONLY support both(RISING and FALLING) EDGE triggle on Internal(GPIO) Interrupt, So it's unsued here*/
        .irq_type = AT91_AIC_SRCTYPE_FALLING, 
    },
};

/* The button platform device private data */
static struct at91_button_platform_data at91_button_data = {
    .buttons = at91_buttons,
    .nbuttons = ARRAY_SIZE(at91_buttons),
};

struct button_device
{
    unsigned char                      *status;      /* The buttons Push down or up status */
    struct at91_button_platform_data   *data;        /* The buttons hardware information data */

    struct timer_list                  *timers;      /* The buttons remove dithering timers */
    wait_queue_head_t                  waitq;           /* Wait queue for poll()  */
    volatile int                       ev_press;     /* Button pressed event */

    struct cdev                        cdev;           
    struct class                       *dev_class; 
} button_device;

static void platform_button_release(struct device * dev)
{
    return; 
}

static struct platform_device at91_button_device = {
    .name    = "at91_button",
    .id      = 1,
    .dev     = 
    {
        .platform_data = &at91_button_data, 
        .release = platform_button_release,
    },
};

static irqreturn_t at91_button_intterupt(int irq,void *de_id)
{
    int i;
    int found = 0;
    struct at91_button_platform_data *pdata = button_device.data;

    //printk("Receive IRQ=%d level=%d status=%d\n", irq, at91_get_gpio_value(irq_to_gpio(irq)),(at91_sys_read(AT91_AIC_SMR(AT91SAM9260_ID_PIOB))>>5)&0x03 );

    for(i=0; i<pdata->nbuttons; i++)
    {
        if(irq == pdata->buttons[i].nIRQ)
        {
            found = 1; 
            break;
        }
    }

    if(!found) /* An ERROR interrupt  */
    {
        return IRQ_NONE;
    }

    /* Only when button record status is up then we will handle this event */
    if(BUTTON_UP  == button_device.status[i])
    {
       button_device.status[i] = BUTTON_UNCERTAIN;
       mod_timer(&(button_device.timers[i]), jiffies + TIMER_DELAY_DOWN);
    }

    return IRQ_HANDLED;
}


static void button_timer_handler(unsigned long data)
{
    struct at91_button_platform_data *pdata = button_device.data;
    int num =(int)data;
    int status = at91_get_gpio_value( pdata->buttons[num].gpio );

    if(LOWLEVEL == status)
    {
        if(BUTTON_UNCERTAIN == button_device.status[num]) /* Come from interrupt */
        {
            //dbg_print("Key pressed!\n");
            button_device.status[num] = BUTTON_DOWN;

            /* Wake up the wait queue for read()/poll() */
            button_device.ev_press = 1;
            wake_up_interruptible(&(button_device.waitq));
        }

        /* Cancel the dithering */
        mod_timer(&(button_device.timers[num]), jiffies + TIMER_DELAY_UP);
    }
    else
    {
        //dbg_print("Key Released!\n");
        button_device.status[num] = BUTTON_UP;
    }

    return ;
}

/*===================== Button device driver part ===========================*/

static int button_open(struct inode *inode, struct file *file)
{ 
    struct button_device *pdev ;
    struct at91_button_platform_data *pdata;
    int i, result;

    pdev = container_of(inode->i_cdev,struct button_device, cdev);
    pdata = pdev->data;
    file->private_data = pdev;

    /* Malloc for all the buttons remove dithering timer */
    pdev->timers = (struct timer_list *) kmalloc(pdata->nbuttons*sizeof(struct timer_list), GFP_KERNEL);
    if(NULL == pdev->timers)
    {
        printk("Alloc %s driver for timers failure.\n", DEV_NAME);
        return -ENOMEM;
    }
    memset(pdev->timers, 0, pdata->nbuttons*sizeof(struct timer_list));

    /* Malloc for all the buttons status buffer */
    pdev->status = (unsigned char *)kmalloc(pdata->nbuttons*sizeof(unsigned char), GFP_KERNEL);
    if(NULL == pdev->status)
    {
        printk("Alloc %s driver for status failure.\n", DEV_NAME);
        result = -ENOMEM; 
        goto  ERROR;
    }
    memset(pdev->status, 0, pdata->nbuttons*sizeof(unsigned char));

    init_waitqueue_head(&(pdev->waitq));

    for(i=0; i<pdata->nbuttons; i++) 
    {
        /* Initialize all the buttons status to UP  */
        pdev->status[i] = BUTTON_UP; 

        /* Initialize all the buttons' remove dithering timer */
        setup_timer(&(pdev->timers[i]), button_timer_handler, i);

        /* Set this PIN to GPIO input mode */
        at91_set_gpio_input(pdata->buttons[i].gpio, ENPULLUP); 

        /* Enable glitch filter for interrupt */
        at91_set_deglitch(pdata->buttons[i].gpio, 1);   

        /* AT91SAM9260 ONLY support both(RISING and FALLING) EDGE triggle, so it's unused here */
#if 0
        irq_set_irq_type(pdata->buttons[i].nIRQ, pdata->buttons[i].irq_type); 
        at91_sys_write(AT91_AIC_SMR(AT91SAM9260_ID_PIOB), AT91_AIC_SRCTYPE_LOW);
        at91_sys_write(AT91_AIC_SMR(AT91SAM9260_ID_PIOB), AT91_AIC_SRCTYPE_FALLING);
        at91_sys_write(AT91_AIC_SMR(AT91SAM9260_ID_PIOB), AT91_AIC_SRCTYPE_HIGH);
        at91_sys_write(AT91_AIC_SMR(AT91SAM9260_ID_PIOB), AT91_AIC_SRCTYPE_RISING);
#endif
        /* Request button GPIO pin interrupt  */
        result = request_irq(pdata->buttons[i].nIRQ, at91_button_intterupt, 0, DEV_NAME, (void *)i);
        if( result )
        {
            result = -EBUSY;
            goto ERROR1;
        }
    }

    return 0;

ERROR1:
     kfree((unsigned char *)pdev->status);
     while(--i) 
     { 
         disable_irq(pdata->buttons[i].nIRQ); 
         free_irq(pdata->buttons[i].nIRQ, (void *)i); 
     }

ERROR:
     kfree(pdev->timers);

     return result;
}

static int button_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{ 
    struct button_device *pdev = file->private_data;
    struct at91_button_platform_data *pdata;
    int   i, ret;
    unsigned int status = 0;

    pdata = pdev->data;

    dbg_print("ev_press: %d\n", pdev->ev_press);
    if(!pdev->ev_press)
    {
         if(file->f_flags & O_NONBLOCK)
         {
             dbg_print("read() without block mode.\n");
             return -EAGAIN;
         }
         else
         {
             /* Read() will be blocked here */
             dbg_print("read() blocked here now.\n");
             wait_event_interruptible(pdev->waitq, pdev->ev_press);
         }
    }

    pdev->ev_press = 0;

    for(i=0; i<pdata->nbuttons; i++)
    {
        dbg_print("button[%d] status=%d\n", i, pdev->status[i]);
        status |= (pdev->status[i]<<i); 
    }

    ret = copy_to_user(buf, (void *)&status, min(sizeof(status), count));

    return ret ? -EFAULT : min(sizeof(status), count);
}

static unsigned int button_poll(struct file *file, poll_table * wait)
{ 
    struct button_device *pdev = file->private_data;
    unsigned int mask = 0;

    poll_wait(file, &(pdev->waitq), wait);
    if(pdev->ev_press)
    {
        mask |= POLLIN | POLLRDNORM; /* The data aviable */ 
    }

    return mask;
}

static int button_release(struct inode *inode, struct file *file)
{ 
    int i;
    struct button_device *pdev = file->private_data;
    struct at91_button_platform_data *pdata;
    pdata = pdev->data;

    for(i=0; i<pdata->nbuttons; i++) 
    {
        disable_irq(pdata->buttons[i].nIRQ);
        free_irq(pdata->buttons[i].nIRQ, (void *)i);
        del_timer(&(pdev->timers[i]));
    }

    kfree(pdev->timers);
    kfree((unsigned char *)pdev->status);

    return 0;
}


static struct file_operations button_fops = { 
    .owner = THIS_MODULE,
    .open = button_open, 
    .read = button_read,
    .poll = button_poll, 
    .release = button_release, 
};


static int at91_button_probe(struct platform_device *dev)
{
    int result = 0;
    dev_t devno;


    /* Alloc the device for driver  */ 
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

    /* Alloc for device major failure */
    if (result < 0) 
    { 
        printk("%s driver can't get major %d\n", DEV_NAME, dev_major); 
        return result; 
    }

    /*  Initialize button_device structure and register cdev*/
     memset(&button_device, 0, sizeof(button_device));
     button_device.data = dev->dev.platform_data;
     cdev_init (&(button_device.cdev), &button_fops);
     button_device.cdev.owner  = THIS_MODULE;

     result = cdev_add (&(button_device.cdev), devno , 1); 
     if (result) 
     { 
         printk (KERN_NOTICE "error %d add %s device", result, DEV_NAME); 
         goto ERROR; 
     }

     button_device.dev_class = class_create(THIS_MODULE, DEV_NAME); 
     if(IS_ERR(button_device.dev_class)) 
     { 
         printk("%s driver create class failture\n",DEV_NAME); 
         result =  -ENOMEM; 
         goto ERROR; 
     }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)     
     device_create(button_device.dev_class, NULL, devno, NULL, DEV_NAME);
#else
     device_create (button_device.dev_class, NULL, devno, DEV_NAME);
#endif

     printk("AT91 %s driver version %d.%d.%d initiliazed.\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER);

     return 0;

ERROR: 
     printk("AT91 %s driver version %d.%d.%d install failure.\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER);
     cdev_del(&(button_device.cdev)); 
     unregister_chrdev_region(devno, 1);
     return result;
}


static int at91_button_remove(struct platform_device *dev)
{
    dev_t devno = MKDEV(dev_major, dev_minor);

    cdev_del(&(button_device.cdev));
    device_destroy(button_device.dev_class, devno);
    class_destroy(button_device.dev_class);

    unregister_chrdev_region(devno, 1); 
    printk("AT91 %s driver removed\n", DEV_NAME);

    return 0;
}


/*===================== Platform Device and driver regist part ===========================*/

static struct platform_driver at91_button_driver = { 
    .probe      = at91_button_probe, 
    .remove     = at91_button_remove, 
    .driver     = { 
        .name       = "at91_button", 
        .owner      = THIS_MODULE, 
    },
};


static int __init at91_button_init(void)
{
   int       ret = 0;

   ret = platform_device_register(&at91_button_device);
   if(ret)
   {
        printk(KERN_ERR "%s: Can't register platform device %d\n", __FUNCTION__, ret); 
        goto fail_reg_plat_dev;
   }
   dbg_print("Regist AT91 %s Device successfully.\n", DEV_NAME);

   ret = platform_driver_register(&at91_button_driver);
   if(ret)
   {
        printk(KERN_ERR "%s: Can't register platform driver %d\n", __FUNCTION__, ret); 
        goto fail_reg_plat_drv;
   }
   dbg_print("Regist AT91 %s Driver successfully.\n", DEV_NAME);

   return 0;

fail_reg_plat_drv:
   platform_driver_unregister(&at91_button_driver);
fail_reg_plat_dev:
   return ret;
}


static void at91_button_exit(void)
{
    platform_driver_unregister(&at91_button_driver);
    dbg_print("AT91 %s platform device removed.\n", DEV_NAME);

    platform_device_unregister(&at91_button_device);
    dbg_print("AT91 %s platform driver removed.\n", DEV_NAME);
}

module_init(at91_button_init);
module_exit(at91_button_exit);

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:AT91SAM9260_button");

