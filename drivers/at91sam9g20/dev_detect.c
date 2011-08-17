/*********************************************************************************
 *  Copyright(c)  2011, GHL Systems Berhad.
 *  All ringhts reserved.
 *
 *     Filename:  dev_detect.c
 *  Description:  GHL netAccess common char device detect driver
 *
 *     ChangLog:
 *      1,   Version: 2.0.0
 *              Date: 2011-05-03
 *            Author: guoqingdong <guoqingdong@ghlsystems.com>
 *       Descrtipion: Initial first version
 *
 ********************************************************************************/

#include "include/plat_driver.h"

#define DRV_AUTHOR                "guoqingdong<guoqingdong@ghlsystems.com>"
#define DRV_DESC                  "GHL netAccess detect module driver"

/*Driver version*/
#define DRV_MAJOR_VER             2
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_DETECT_NAME

#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*dynamic major by default */
#endif

#define USB_VBUS_CTL_PIN    AT91_PIN_PB22

int card_state;
int eth0_link_state;
int eth1_link_state;
int usb_state;
u16 ralinkID;

wait_queue_head_t wait_if_state_changed;

static int prv_card_state = 0;
static int prv_eth0_link_state = 0;
static int prv_eth1_link_state = 0;
static int prv_usb_state = 0;
static u16 prv_wifi_state = 0;

static DEFINE_SPINLOCK(spin);

static struct semaphore sem;

enum IF_ID_T {
	sd = 0,
	eth0,
	eth1,
	usb,
	wifi,
	max_if_id
} if_id;

static int if_state[max_if_id];

static struct cdev *dev_cdev = NULL;
static struct class *dev_class = NULL;

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

#define dbg_print(format,args...) if(DISABLE!=debug) \
                                  {printk("[kernel] ");printk(format, ##args);}
#ifdef CONFIG_PROC_FS
static int ioctl_proc_read(char *page, char **start, off_t offset, int count, int *eof, void *data)
{
    char *p = page;
    if (0 != offset)
    {
        *eof = 1;
        return 0;
    }

    p += sprintf(p, "Current driver support ioctl command:\n");
    {
        p += sprintf(p, "========================== Commone ioctl =============================\n");
        p += sprintf(p, "Enable/Disable Debug    : %u\n", SET_DRV_DEBUG);
        p += sprintf(p, "Get driver version      : %u\n", GET_DRV_VER);
        p += sprintf(p, "\n");

        p += sprintf(p, "=========================== LED driver ==============================\n");
        p += sprintf(p, "Turn LED on             : %u\n", LED_ON);
        p += sprintf(p, "Turn LED off            : %u\n", LED_OFF);
        p += sprintf(p, "Turn LED blink          : %u\n", LED_BLINK);
        p += sprintf(p, "Turn all LED on         : %u\n", ALL_LED_ON);
        p += sprintf(p, "Turn all LED off        : %u\n", ALL_LED_OFF);
        p += sprintf(p, "Turn all LED blink      : %u\n", ALL_LED_BLINK);
        p += sprintf(p, "\n");

        p += sprintf(p, "=========================== Beep driver ==============================\n");
        p += sprintf(p, "Turn buzzer on          : %u\n", BEEP_ENALARM);
        p += sprintf(p, "Turn buzzer off         : %u\n", BEEP_DISALARM);
		p += sprintf(p, "Set buzzer frequency    : %u\n", SET_DEFAULT_BEEP_FREQ);
		
        p += sprintf(p, "\n");

        p += sprintf(p, "=========================== RS485 driver =============================\n");
        p += sprintf(p, "Set RS485 as send mode  : %u\n", RS485_DIR_SEND);
        p += sprintf(p, "Set RS485 as recv mode  : %u\n", RS485_DIR_RECV);
        p += sprintf(p, "\n");

        p += sprintf(p, "=========================== Modem driver =============================\n");
        p += sprintf(p, "Modem reset             : %u\n", MODEM_RESET);
        p += sprintf(p, "Set modem DTR pin       : %u\n", MODEM_SET_DTR);
        p += sprintf(p, "Set modem RTS pin       : %u\n", MODEM_SET_RTS);
        p += sprintf(p, "Get modem DCD pin       : %u\n", MODEM_GET_DCD);
        p += sprintf(p, "\n");


        p += sprintf(p, "=================== Hardware status Detect driver ====================\n");
        p += sprintf(p, "Detect WIFI module ID   : %u\n", DETECT_WIFI_ID);
		p += sprintf(p, "Detect SD/ETH0/ETH1/WIFI: %u\n", DETECT_STATUS);
		p += sprintf(p, "Detect SAM Card present : %u\n", SAM_PRESENT_DETECT);
        p += sprintf(p, "\n");

        p += sprintf(p, "=========================== GPRS driver ==============================\n");
        p += sprintf(p, "GPRS power on           : %u\n", GPRS_POWERON);
        p += sprintf(p, "GPRS power off          : %u\n", GPRS_POWERDOWN);
        p += sprintf(p, "GPRS(3G) reset          : %u\n", GPRS_RESET);
        p += sprintf(p, "GPRS(3G) power monitor  : %u\n", GPRS_POWERMON);
        p += sprintf(p, "Check SIM door          : %u\n", GPRS_CHK_SIMDOOR);
        p += sprintf(p, "Set work SIM slot       : %u\n", SET_WORK_SIMSLOT);
        p += sprintf(p, "Get working SIM slot    : %u\n", CHK_WORK_SIMSLOT);
        p += sprintf(p, "Set GPRS DTR pin        : %u\n", GPRS_SET_DTR);
        p += sprintf(p, "Set GPRS RTS pin        : %u\n", GPRS_SET_RTS);
        p += sprintf(p, "Check GPRS model        : %u\n", GPRS_CHK_MODEL);
        p += sprintf(p, "\n");
    }

    return (p - page);
}

#define PROC_IOCTL       "ioctl"
#endif
								  
static int detect_open( struct inode *inode, struct file *file )
{
	if (down_interruptible(&sem)) 
	{
		return -ERESTARTSYS;
	}	
	
	spin_lock(&spin);
	prv_card_state = -1;
	prv_eth0_link_state = -1;
	prv_eth1_link_state = -1;	
	prv_usb_state = -1;
	prv_wifi_state = 0;
	spin_unlock(&spin);
	
	up(&sem);
	
	return 0;
}

static int detect_release( struct inode *inode, struct file *file )
{
	return 0;
}

static int detect_read( struct file *filp, char *buf, size_t len, loff_t *off)
{
	if (down_interruptible(&sem)) 
	{
		return -ERESTARTSYS;
	}
		
	wait_event_interruptible(wait_if_state_changed, 
					   card_state!=prv_card_state           \
	 				|| eth0_link_state!=prv_eth0_link_state \
	 				|| eth1_link_state!=prv_eth1_link_state \
	 				|| usb_state!=prv_usb_state             \
	 				|| ralinkID!=prv_wifi_state);
	
	if_state[sd] = card_state;
	if_state[eth0] = eth0_link_state;
	if_state[eth1] = eth1_link_state;
	if_state[usb] = usb_state;
	if_state[wifi] = ralinkID;
	if (copy_to_user(buf, if_state, sizeof(if_state))) 
	{
		up(&sem);
		return -EFAULT;
	}
	
	up(&sem);
	
	return 0;
}

static long detect_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
		case SET_DRV_DEBUG_OLD:
		case SET_DRV_DEBUG:
			dbg_print("%s driver debug now.\n", DISABLE == arg ? "Disable" : "Enable");

			if (0 == arg)
				debug = DISABLE;
			else
				debug = ENABLE;

			break;

		case GET_DRV_VER_OLD:
		case GET_DRV_VER:
			print_version(DRV_VERSION);
			return DRV_VERSION;

		case DETECT_WIFI_ID_OLD:
		case DETECT_WIFI_ID:
			if (down_interruptible(&sem)) 
			{
				return -ERESTARTSYS;
			}
#if (defined HWVER_V22)
			if(ralinkID == 0 && 0x01 == at91_get_gpio_value(USB_VBUS_CTL_PIN))
			{
			    at91_set_gpio_output(USB_VBUS_CTL_PIN, 0);
			    SLEEP(800);
			    at91_set_gpio_output(USB_VBUS_CTL_PIN, 1);
			}
#endif
			printk(KERN_INFO "WIFI ID:0x%x\n",ralinkID);
			up(&sem);
			break;
			
		case DETECT_STATUS_OLD:
		case DETECT_STATUS:
			if (down_interruptible(&sem)) 
			{
				return -ERESTARTSYS;
			}
			if_state[sd] = card_state;
			if_state[eth0] = eth0_link_state;
			if_state[eth1] = eth1_link_state;
			if_state[usb] = usb_state;
			if_state[wifi] = ralinkID;

			if (copy_to_user((void *)arg, (void *)if_state, sizeof(if_state))) 
			{
				up(&sem);
				return -EFAULT;
			}
			up(&sem);
			break;
	
      default:
          printk("%s driver don't support ioctl command=%d\n", DEV_NAME, cmd);
          return -1;
    }
    return 0;
}

static unsigned int detect_poll( struct file *filp, poll_table *tabp )
{
	unsigned int mask = 0;
	
	poll_wait(filp, &wait_if_state_changed, tabp);
	
	if (card_state!=prv_card_state
		 || eth0_link_state!=prv_eth0_link_state
		 || eth1_link_state!=prv_eth1_link_state
		 || usb_state!=prv_usb_state
		 || ralinkID!=prv_wifi_state) {
		 	
		spin_lock(&spin);
		mask |= POLLIN | POLLRDNORM;
		prv_card_state = card_state;
		prv_eth0_link_state = eth0_link_state;
		prv_eth1_link_state = eth1_link_state;
		prv_usb_state = usb_state;
		prv_wifi_state = ralinkID;
		spin_unlock(&spin);		
	}
	return mask;
}

static struct file_operations detect_fops = {
    .owner = THIS_MODULE,
    .open = detect_open,
    .release = detect_release,
    .read = detect_read,
    .unlocked_ioctl = detect_ioctl,
    .poll = detect_poll,
};

static void detect_cleanup(void)
{
    dev_t devno = MKDEV(dev_major, dev_minor);

#ifdef CONFIG_PROC_FS    
	remove_proc_entry(PROC_IOCTL, NULL);    
#endif

    device_destroy(dev_class, devno);   
    class_destroy(dev_class);

    cdev_del(dev_cdev);
    unregister_chrdev_region(devno, 1);

    printk("%s driver removed\n", DEV_NAME);
}

static int __init detect_init(void)
{
    int result;
    dev_t devno;

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
        printk("%s driver can't alloc for detect_cdev\n", DEV_NAME);
        goto ERROR;
    }

    /*Initialize cdev structure and register it */
    dev_cdev->owner = THIS_MODULE;
    dev_cdev->ops = &detect_fops;
    result = cdev_add(dev_cdev, devno, 1);
    if (0 != result)
    {
        printk("%s driver can't alloc for detect_cdev\n", DEV_NAME);
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
	create_proc_read_entry(PROC_IOCTL, 0444, NULL, ioctl_proc_read, NULL);    
	printk("Initializ proc file \"/proc/%s\"\n", PROC_IOCTL);
#endif

    sema_init(&sem, 1);
    printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER,
           DRV_REVER_VER);

    return 0;

ERROR:
    cdev_del(dev_cdev);
    unregister_chrdev_region(devno, 1);
    return result;
}

module_init(detect_init);
module_exit(detect_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);


