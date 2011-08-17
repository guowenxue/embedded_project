/*********************************************************************************
 *  Copyright(c)  2011, Guo Wenxue <guowenxue@gmail.com>
 *  All ringhts reserved.
 *
 *     Filename:  dev_eeprom.c
 *  Description:  AT91SAM9XXX platform EEPROM driver
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
#define DRV_DESC                  "AT91SAM9XXX platform EEPROM driver"

/*Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_EEPROM_NAME

#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*dynamic major by default */
#endif

#define LL_PWRON_CLKS  15 // Number of clocks to reset CM on power up
#define LL_ACK_TRIES    8 // Number of times to try to get ACK on a write

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

// input/output to adr on long type: 32bit doble word
#define inpl(adr) ( *((unsigned long volatile *)(adr)) )
#define outpl(adr,val) ( *((unsigned long volatile *)(adr)) = val )

volatile unsigned int *TWI_CR;
volatile unsigned int *TWI_CWGR;
volatile unsigned int *TWI_IDR;
volatile unsigned int *PMC_PCER;

/* Sets the clock pin to a high level by floating the port output */
static void ll_Clockhigh(void)
{
    udelay(1);

    at91_set_gpio_input(TWI_SCLK, DISPULLUP);
    at91_set_gpio_value(TWI_SCLK, HIGHLEVEL);

    udelay(1);
}

/* Sets the clock pin to a low level.  */
static void ll_Clocklow(void)
{

    udelay(1);
    at91_set_gpio_value(TWI_SCLK, LOWLEVEL);

    udelay(2);
    at91_set_gpio_output(TWI_SCLK, LOWLEVEL);
}

/* Cycles the clock pin low then high. */
static void ll_ClockCycle(void)
{
    ll_Clocklow();
    ll_Clockhigh();
}

/* Calls ll_ClockCycle() ucCount times */
static void ll_ClockCycles(unsigned char ucCount)
{
    for ( ; ucCount > 0 ; ucCount-- )
        ll_ClockCycle() ;
}

/*  Sets the data pin to a high level by floating the port output
 *  Uses the internal port pull-up to provide the TWI data pull-up.
 */
static void ll_Datahigh(void)
{
    udelay(1);

    at91_set_gpio_input(TWI_SDAT, DISPULLUP);
    at91_set_gpio_value(TWI_SDAT, HIGHLEVEL);

    udelay(2);
}

/* Sets the data pin to a low level.  */
static void ll_Datalow(void)
{
    udelay(1);
    at91_set_gpio_value(TWI_SDAT, LOWLEVEL);

    udelay(2);
    at91_set_gpio_output(TWI_SDAT, LOWLEVEL);
}

/* Reads and returns the data pin value.  Leaves the pin high-impedance. */
static unsigned char ll_Data(void)
{
    udelay(1);

    at91_set_gpio_input(TWI_SDAT, DISPULLUP);
    at91_set_gpio_value(TWI_SDAT, HIGHLEVEL);

    udelay(4);

    return at91_get_gpio_value(TWI_SDAT);
}

/* Sends a start sequence */
static void ll_Start(void)
{
          ll_Clocklow();
          ll_Datahigh();
          udelay(4);
          ll_Clockhigh();
          udelay(4);
          ll_Datalow();
          udelay(4);
          ll_Clocklow();
          udelay(4);
}

/* Sends a stop sequence */
static void ll_Stop(void)
{
        ll_Clocklow();
        ll_Datalow();
        ll_Clockhigh();
        udelay(8);
        ll_Datahigh();
        udelay(4);
}


/*  Sends an ACK or NAK to the device (after a read). 
 *  param ucAck - if ::TRUE means an ACK will be sent, otherwise NACK sent.
 */
static void ll_AckNak(unsigned char ucAck)
{
        ll_Clocklow();
        if (ucAck)
    {
            ll_Datalow() ; //Low data line indicates an ACK
    }
        else
    {
        ll_Datahigh(); // High data line indicates an NACK
    }
        ll_Clockhigh();
        ll_Clocklow();
}

/*  Power On chip sequencing.  Clocks the chip ::LL_PWRON_CLKS times.  */
static void ll_PowerOn(void)
{
    ll_Datahigh() ;    // Data high during reset
    ll_Clocklow() ;    // Clock should start LOWLEVEL
    ll_ClockCycles(LL_PWRON_CLKS);
}

/* power off chip sequencing */
static void ll_PowerOff(void)
{
    udelay(1);
    ll_Clocklow() ;
    udelay(6);
}

/* Write a byte on the TWI-like bus.
*
*  return 0 if write success, else 'n' (the number of attempts) 
*  to get ACK that failed (maximum is ::LL_ACK_TRIES).
*/
static unsigned char ll_Write(unsigned char ucData)
{
    unsigned char i;

    for(i=0; i<8; i++)
    {                 // Send 8 bits of data
        ll_Clocklow();
        if (ucData&0x80)
            ll_Datahigh();
        else
            ll_Datalow() ;
        ll_Clockhigh();
        ucData = ucData<<1;
    }

    ll_Clocklow();
    mdelay(2);

    // wait for the ack
    ll_Datahigh(); // Set data line to be an input
    udelay(8);
    ll_Clockhigh();
    for ( i = 0 ; i < LL_ACK_TRIES ; i++ )
    {
    if ( ll_Data() == 0 )
        {   i = 0 ;
            break ;
        }
    }
    ll_Clocklow();

    return i;
}

/* Read a byte from device, MSB first, no ACK written yet*/
static unsigned char ll_Read(void)
{
    unsigned char i;
    unsigned char rByte = 0;

    ll_Datahigh();
    for(i=0x80; i; i=i>>1)
    {
        ll_ClockCycle();

        if (ll_Data())
        rByte |= i;

        ll_Clocklow();
    }

    return rByte;
}

static int eeprom_status = 0;

static int eeprom_open(struct inode *inode, struct file *file)
{
	if(eeprom_status==0)
	{
		eeprom_status = 1;
		return 0;
	}
	printk("\n\n new L350EepromDrv open\n\n");
	return -1;
}

static int eeprom_release(struct inode *inode, struct file *file)
{
	eeprom_status = 0;
    return 0;
}

//compatible with kernel version 2.6.38 
static long eeprom_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned char parameter;
	unsigned char i,j;
    copy_from_user((char *)&parameter,(char *)arg,1);
	
    switch(cmd)
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

		case LL_POWERON:
			ll_PowerOn();
			break;

        case LL_POWEROFF:
            ll_PowerOff();
            break;

        case LL_DATALOW:
            ll_Datalow();
            for(j=0; j<parameter; j++)
            {
	            ll_Start();
	            for(i = 0; i<15; i++)
                {
                    ll_ClockCycle();
                }
                ll_Stop();
            }
            break;

        case LL_STOP:
            ll_Stop();
            break;

        case LL_ACKNAK:
            ll_AckNak(parameter);
            break;

        case LL_START:
            ll_Start();
            break;

        case LL_WRITE:
            i=ll_Write(parameter);
            if(copy_to_user((char *)arg,(char *)&i,1))
            {
	            return -1;
            }
            break;

        case LL_READ:
            parameter = ll_Read();
	        if(copy_to_user((char *)arg,(char *)&parameter,1))
            {
                return -1;
            }
            break;

      default:
          printk("%s driver don't support ioctl command=%d\n", DEV_NAME, cmd);
          return -1;	
    }
    return 0;
}

static struct file_operations eeprom_fops = {
    .owner = THIS_MODULE,
    .open = eeprom_open,
    .release = eeprom_release,
    .unlocked_ioctl = eeprom_ioctl,  //compatible with kernel version 2.6.38 
};

static void eeprom_cleanup(void)
{
    dev_t devno = MKDEV(dev_major, dev_minor);

    device_destroy(dev_class, devno);   
    class_destroy(dev_class);

    cdev_del(dev_cdev);
    unregister_chrdev_region(devno, 1);

    printk("%s driver removed\n", DEV_NAME);
}

static int __init eeprom_init(void)
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
		unregister_chrdev_region(devno, 1);
        printk("%s driver can't alloc for eeprom_cdev\n", DEV_NAME);
        return -ENOMEM;
    }

    /*Initialize cdev structure and register it */
    dev_cdev->owner = THIS_MODULE;
    dev_cdev->ops = &eeprom_fops;
    result = cdev_add(dev_cdev, devno, 1);
    if (0 != result)
    {
        printk("%s driver can't alloc for eeprom_cdev\n", DEV_NAME);
        goto ERROR;
    }

	dev_class = class_create(THIS_MODULE, DEV_NAME);
    if(IS_ERR(dev_class)) 
    {           
        printk("%s driver create class failture\n", DEV_NAME);           
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

module_init(eeprom_init);
module_exit(eeprom_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);


