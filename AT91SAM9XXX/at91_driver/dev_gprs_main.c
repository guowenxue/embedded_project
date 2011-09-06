/*********************************************************************************
 *  Copyright(c)  2011, Guo Wenxue <guowenxue@gmail.com>
 *  All ringhts reserved.
 *
 *     Filename:  dev_gprs.c
 *  Description:  AT91SAM9XXX GPRS/3G module driver
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-08-10
 *            Author: Guo Wenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 ********************************************************************************/
#include "dev_gprs.h"

int            debug        =  DISABLE;

GSM_DEVICE     *current_dev =  NULL;

static struct class *dev_class;
static struct cdev dev_cdev;

static int dev_major = DEV_MAJOR;
static int dev_minor = 0;

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

static struct mutex     mutex; /* Used to lock g_ucRing */
static unsigned char    g_ucRing = RING_NONE;   /* 0x01 Means SMS, 0X02 Means Incoming call */
int curr_gprs_id = GSM_UNKNOW;


//only check in uc864e
static irqreturn_t catchRing_irq (int irq, void *dev_id)
{
	int                     value;
	static unsigned long    msec = 0;
	static unsigned long    cycle_start = 0;

	value = at91_get_gpio_value(GPRS_RI_PIN);
	if(LOWLEVEL == value)  /* It's a FALLING EDGE */
	{
		msec = jiffies;     /* FALLING to lowlevel time start */
	}
	else if(HIGHLEVEL==value) /* It's a RISING EDGE */
	{
		if(time_before(jiffies, cycle_start))
		{
			cycle_start = jiffies+660;
			return IRQ_NONE; /* It's the incoming call continue RING tone, skip it */
		}

		msec = jiffies - msec;   /* How long time the low level hold */
		if(11<msec &&  msec< 17 )  /* 14ms means an incoming SMS */
		{
			dbg_print("RING for %lums, It's an Incoming SMS.\n", msec);
			mutex_lock(&mutex);
			g_ucRing = RING_SMS;
			mutex_unlock(&mutex);
		}
		else if(95<msec && msec<105) /* 100ms means an incoming call */
		{
			dbg_print("RING for %lums, It's an Incoming Call.\n", msec); 
			mutex_lock(&mutex);
			g_ucRing = RING_CALL;
			mutex_unlock(&mutex);
		}
		cycle_start = jiffies+660;
	}
	return IRQ_HANDLED;
}

static int gprs_open(struct inode *inode, struct file *filp)
{
    if (request_irq (GPRS_RI_PIN, catchRing_irq, IRQ_TYPE_EDGE_BOTH | IRQF_DISABLED, "gprs_ring", NULL))
    {
		printk(KERN_WARNING "Request GPRS RING irq failed\n" );
		return -ENODEV;
    }

    dbg_print("Open \"%s\" GPRS module\n", support_gprs[curr_gprs_id].name);
    return 0;                   /* success */
}

static int gprs_release(struct inode *inode, struct file *file)
{
	free_irq (GPRS_RI_PIN, NULL);
    return 0;
}

static long gprs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int iRet = 0;	
    
    dbg_print("ioctl() on %s driver with cmd=%u arg=%lu\n", support_gprs[curr_gprs_id].name, cmd, arg);

    switch (cmd)
    {
		case SET_DRV_DEBUG:
			dbg_print("%s driver debug now.\n", DISABLE==arg?"Disable":"Enable");

			if (0 == arg)
			  debug = DISABLE;
			else
			  debug = ENABLE;

			break;

		case GET_DRV_VER:
			print_version(DRV_VERSION);
			return DRV_VERSION;

#ifdef PLAT_L2
        case GPRS_CHK_MODEL:
            return detect_gprs_model();
#endif

		case GPRS_POWERON:
			gprs_set_worksim(arg);
			gprs_powerup(curr_gprs_id);
			break;

		case GPRS_POWERDOWN:
			gprs_powerdown(curr_gprs_id);
			break;

		case GPRS_RESET:         /*Only 3G module support */
			gprs_reset(curr_gprs_id);
			break;

		case GPRS_CHK_SIMDOOR:
			iRet = gprs_chk_simdoor(arg);
			break;

		case CHK_WORK_SIMSLOT:
			iRet = gprs_get_worksim();
			break;

		case GPRS_SET_DTR:
			gprs_set_dtr(curr_gprs_id,arg);
			break;
	
        case GPRS_GET_RING:
			iRet = g_ucRing;     /* Save the Return value */
			mutex_lock(&mutex);
			g_ucRing = RING_NONE;  /* Clear the incoming RING */
			mutex_unlock(&mutex);
			dbg_print("Get Ring Call: %d\n", g_ucRing);

			return iRet;

#ifndef PLAT_L2
		case GPRS_POWERMON:      /*Only 3G module support */
			iRet = gprs_powermon(curr_gprs_id);
			break;

		case SET_WORK_SIMSLOT:
			iRet = gprs_set_worksim(arg);
			break;

		case GPRS_SET_RTS:
			gprs_set_rts(curr_gprs_id,arg);
			break;
#endif
	
        default:
			printk("%s driver don't support ioctl command=%u\n", DEV_NAME, cmd);
			return -1;
	}
    return iRet;
}

#ifdef CONFIG_PROC_FS
static int read_proc_gprs(char *page, char **start, off_t offset, int count, int *eof, void *data)
{
    char *p = page;
    if (0 != offset)
    {
        *eof = 1;
        return 0;
    }

    p += sprintf(p, "Current on board GPRS module: %s\n", support_gprs[curr_gprs_id].name);
    p += sprintf(p, "Power on period time        : %lu\n", support_gprs[curr_gprs_id].poweron_period_time);
    p += sprintf(p, "Power on AT active time     : %lu\n", support_gprs[curr_gprs_id].atcmd_active_time);
    p += sprintf(p, "Power off period time       : %lu\n", support_gprs[curr_gprs_id].poweroff_period_time);
    p += sprintf(p, "Power oiff AT inactive time : %lu\n", support_gprs[curr_gprs_id].atcmd_inactive_time);
    p += sprintf(p, "Ring call period time       : %lu\n", support_gprs[curr_gprs_id].ring_call_time);
    p += sprintf(p, "Ring SMS period time        : %lu\n", support_gprs[curr_gprs_id].ring_sms_time);
    p += sprintf(p, "\n");

    return (p - page);
}
#endif

struct file_operations gprs_fops = {
	.owner = THIS_MODULE,
	.open = gprs_open,
	.unlocked_ioctl = gprs_ioctl,
	.release = gprs_release,
};

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
static void gprs_cleanup(void)
{    
	dev_t devno = MKDEV(dev_major, dev_minor);
	
    gprs_powerdown(curr_gprs_id);
#ifdef CONFIG_PROC_FS    
	remove_proc_entry(FILESYS_GPRS, NULL);
#endif    

    device_destroy(dev_class, devno);
    class_destroy (dev_class);  
    
    cdev_del(&dev_cdev);
	
    unregister_chrdev_region(devno, 1);
	printk (KERN_INFO "%s driver removed\n",DEV_NAME);
}


static int gprs_init(void)
{
	int result;
	dev_t devno;

    if(GSM_UNKNOW == (curr_gprs_id=detect_gprs_model()))
    {
          printk("ERROR: Can not detect any GPRS module on board, GPRS driver install failure.\n"); 
          return -ENODEV;
    }

    gprs_hw_init(curr_gprs_id);
	mutex_init(&mutex);
	
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
	if (result<0) 
	{
		printk (KERN_WARNING "%s driver: can't get major number %d\n", DEV_NAME, dev_major);
		return result;
	}

    /*Initialize cdev structure and register it*/
    cdev_init (&dev_cdev, &gprs_fops);
    dev_cdev.owner  = THIS_MODULE;

    result = cdev_add (&dev_cdev, devno , 1);
    if (result)
    {
        printk (KERN_NOTICE "error %d add GPRS device", result);
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


#ifdef CONFIG_PROC_FS
    create_proc_read_entry(DEV_NAME, 0444, NULL, read_proc_gprs, NULL);
#endif

    printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER);

    return 0;

ERROR:
    cdev_del(&dev_cdev);
    unregister_chrdev_region(devno, 1);
    return result;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);

module_init(gprs_init);
module_exit(gprs_cleanup);



