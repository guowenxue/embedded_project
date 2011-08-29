/*********************************************************************************
 *  Copyright(c)  2011, Guo Wenxue <guowenxue@gmail.com>
 *  All ringhts reserved.
 *
 *     Filename:  dev_gpio.c
 *  Description:  AT91SAM9XXX Key pad driver
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-08-10
 *            Author: Guo Wenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 ********************************************************************************/

#include "include/plat_driver.h"

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9XXX Key pad driver"

/*Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_KEY_NAME

#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*dynamic major by default */
#endif

static struct cdev dev_cdev;
static struct class *dev_class = NULL;

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

#define dbg_print(format,args...) if(DISABLE!=debug) \
        {printk("[kernel] ");printk(format, ##args);}    

static unsigned long  tick;
static int  pressed = 0x00;
static int  sec;

static irqreturn_t keypad_handler (int irq, void *dev_id)
{
   int value;

   at91_sys_write (AT91_AIC_ICCR, 1 << AT91SAM9260_ID_PIOB);    // clear interrupt

   value = at91_get_gpio_value(RESTORE_KEY);
   if(RESTR_KEY_DOWN == value)
   {
        tick = jiffies;
        pressed = 0x01;
        sec = 0;
   }
   else 
   {
        /*Sometimes, it will miss some Key pressed or release action, use release to fix the bug*/
        if(0x01==pressed)
        {
           tick = jiffies - tick;
           sec = tick / HZ;
        }
        pressed = 0x00;
   }

   return IRQ_HANDLED;
}

#ifdef FUNC_SWITCH_SUPPORT
static int detect_func_key(void)
{
    int value = 0;

    if(DISABLE!=debug)
    {
       printk("FUNC_SWITCH_PIN0 Key GPIO level: %d\n", at91_get_gpio_value(FUNC_SWITCH_PIN0));
       printk("FUNC_SWITCH_PIN1 Key GPIO level: %d\n", at91_get_gpio_value(FUNC_SWITCH_PIN1));
       printk("FUNC_SWITCH_PIN2 Key GPIO level: %d\n", at91_get_gpio_value(FUNC_SWITCH_PIN2));
       printk("FUNC_SWITCH_PIN3 Key GPIO level: %d\n", at91_get_gpio_value(FUNC_SWITCH_PIN3));
    }
    value = at91_get_gpio_value (FUNC_SWITCH_PIN0);
    value |= (at91_get_gpio_value (FUNC_SWITCH_PIN1)<<1);
    value |= (at91_get_gpio_value (FUNC_SWITCH_PIN2)<<2);
    value |= (at91_get_gpio_value (FUNC_SWITCH_PIN3)<<3);

    return value;
}
#endif

static int keypad_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int keypad_release(struct inode *inode, struct file *file)
{
    return 0;
}

//compatible with kernel version 2.6.38 
static long keypad_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
    {
		case SET_DRV_DEBUG:
			dbg_print("%s driver debug now.\n", DISABLE == arg ? "Disable" : "Enable");
			if (0 == arg)
				debug = DISABLE;
			else
				debug = ENABLE;
			break;
        
        case DETECT_RESTORE_KEY:
            return pressed;

#ifdef FUNC_SWITCH_SUPPORT
        case DETECT_FUNC_KEY:
            return detect_func_key();
#endif

		default:
			dbg_print("%s driver don't support ioctl command=%d\n", DEV_NAME, cmd);
			return -1;
    }

    return 0;
}

static struct file_operations keypad_fops = {
    .owner = THIS_MODULE,
    .open = keypad_open,
    .release = keypad_release,
    .unlocked_ioctl = keypad_ioctl, //compatible with kernel version 2.6.38 
};


static void keypad_cleanup(void)
{
    dev_t devno = MKDEV(dev_major, dev_minor);

    free_irq (RESTORE_KEY, NULL);

    device_destroy(dev_class, devno);   
    class_destroy(dev_class);

    cdev_del(&dev_cdev);

    unregister_chrdev_region(devno, 1);

    printk("%s driver removed\n", DEV_NAME);
}

static int __init keypad_init(void)
{
    int result;
    dev_t devno;

    at91_set_gpio_input(RESTORE_KEY, 0); /*Remove the first interrupt when insmod*/
    at91_set_deglitch(RESTORE_KEY, 1);
    if (request_irq (RESTORE_KEY, keypad_handler, 0, "Keypad", NULL))
    {
         printk(KERN_WARNING "Request Restore Key IRQ failed\n" ); 
    }

    /* Alloc for the device for driver */
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
	
    /*Initialize cdev structure and register it*/
	cdev_init (&dev_cdev, &keypad_fops);
	dev_cdev.owner 	= THIS_MODULE;
	
	result = cdev_add (&dev_cdev, devno , 1);
	if (result)
	{
	    printk (KERN_NOTICE "error %d add GPIO device", result);
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
    cdev_del(&dev_cdev);
    unregister_chrdev_region(devno, 1);
    return result;
}

module_init(keypad_init);
module_exit(keypad_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);


