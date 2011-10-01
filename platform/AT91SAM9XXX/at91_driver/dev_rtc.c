/*********************************************************************************
 *  Copyright(c)  2011, Guo Wenxue <guowenxue@gmail.com>
 *  All ringhts reserved.
 *
 *     Filename:  dev_rtc.c
 *  Description:  AT91SAM9XXX GPIO sinulator I2C bus RTC driver
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *           Date: 2011-08-10
 *           Author: Guo Wenxue <guowenxue@gmail.com>
 *           Descrtipion: Initial first version
 *
 ********************************************************************************/

#include "include/plat_driver.h"
#include <linux/rtc.h>
#include <linux/platform_device.h>

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9XXX RTC driver"

/*Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  "rtc-simu"

#define SDA  AT91_PIN_PA23
#define SCL  AT91_PIN_PA24

#define SET_SDA_OUT_HIGH   at91_set_gpio_output(SDA, 1)
#define SET_SDA_OUT_LOW    at91_set_gpio_output(SDA, 0)
#define SET_SCL_OUT_HIGH   at91_set_gpio_output(SCL, 1)
#define SET_SCL_OUT_LOW    at91_set_gpio_output(SCL, 0)
#define GET_SDA            at91_get_gpio_value(SDA)
#define I2C_WAIT           udelay(25)
#define CFG_SDA_IN         at91_set_gpio_input(SDA, 0);\
                           at91_set_multi_drive(SDA, 1)

/* RTC registers don't differ much, except for the century flag */
#define DS1307_REG_SECS        0x00 /* 00-59 */
#define DS1307_BIT_CH          0x80
#define DS1307_REG_MIN         0x01 /* 00-59 */
#define DS1307_REG_HOUR        0x02 /* 00-23, or 1-12{am,pm} */
#define DS1307_REG_WDAY        0x03 /* 01-07 */
#define DS1307_REG_MDAY        0x04 /* 01-31 */
#define DS1307_REG_MONTH       0x05 /* 01-12 */
#define DS1337_BIT_CENTURY     0x80 /* in REG_MONTH */
#define DS1307_REG_YEAR        0x06 /* 00-99 */

static struct platform_device *gpio_rtc_dev = NULL;

static struct proc_dir_entry * rtc_simu;

static void Start_I2c(void)
{
     SET_SDA_OUT_HIGH;   
     I2C_WAIT;
     SET_SCL_OUT_HIGH;
     I2C_WAIT;   

     SET_SDA_OUT_LOW;  
     I2C_WAIT;   
     SET_SCL_OUT_LOW; 
     I2C_WAIT;
}

static void Stop_I2c(void)
{
     SET_SDA_OUT_LOW; 
     I2C_WAIT;   
     SET_SCL_OUT_HIGH; 
     I2C_WAIT;
     SET_SDA_OUT_HIGH; 
     I2C_WAIT;
}

static unsigned char SendByte(unsigned char c)
{
     unsigned char BitCnt;
     unsigned char ack; 
 
     for (BitCnt=0; BitCnt<8; BitCnt++) 
     {
	  if ( (c<<BitCnt) & 0x80 )
	       SET_SDA_OUT_HIGH;   
	  else  
	       SET_SDA_OUT_LOW;                

	  I2C_WAIT;
	  SET_SCL_OUT_HIGH;        

	  I2C_WAIT; 
	  SET_SCL_OUT_LOW; 
     }
    
     I2C_WAIT;
     SET_SDA_OUT_HIGH;             
     I2C_WAIT;   
     SET_SCL_OUT_HIGH;
     I2C_WAIT;

///     CFG_SDA_IN;
     if(GET_SDA == 1)
	  ack=0;
     else
 	  ack=1;        
     SET_SCL_OUT_LOW;
     I2C_WAIT;

     return ack;
}

 
static unsigned char  RcvByte(void)
{
     unsigned char retc;
     unsigned char BitCnt;
     
     retc = 0; 
     SET_SDA_OUT_HIGH;            
     for ( BitCnt=0; BitCnt<8; BitCnt++ )
     {
	  I2C_WAIT;           
	  SET_SCL_OUT_LOW;      
	  I2C_WAIT;
	  SET_SCL_OUT_HIGH; 
	  I2C_WAIT;
	  retc=retc<<1;
	  ///  CFG_SDA_IN;

	  if(GET_SDA==1)
	       retc=retc+1; 
	  I2C_WAIT; 
     }

     SET_SCL_OUT_LOW;    
     I2C_WAIT;
     I2C_WAIT;

     return(retc);
}
 
static void Ack_I2c(unsigned char a)
{
  
     if(a==0)
	  SET_SDA_OUT_LOW;   
     else 
	  SET_SDA_OUT_HIGH;
     I2C_WAIT;      

     SET_SCL_OUT_HIGH;
     I2C_WAIT;  

     SET_SCL_OUT_LOW;        
     I2C_WAIT;    
}

/* static unsigned char ISendByte(unsigned char sla,unsigned char c) */
/* { */
/*      Start_I2c();             */

/*      if ( 0 == SendByte(sla) )  */
/*      { */
/* 	  return (0); */
/*      } */
/*      if ( 0 == SendByte(c) ) */
/*      { */
/* 	  return (0); */
/*      } */

/*      Stop_I2c();                */
/*      return(1); */
/* } */

/* Success: return 1
   Fail   : return 0 */
static unsigned char ISendStr(unsigned char sla,unsigned char suba,unsigned char *s,unsigned char no)
{
     unsigned char i;
     
     Start_I2c();              
     if ( 0 == SendByte(sla) ) 
     {
	  return (0);
     }
     if ( 0 == SendByte(suba) )
     {
	  return (0);
     }

     for(i=0;i<no;i++)
     {   
	  if ( 0 == SendByte(*s) )
	  {
	       return (0);
	  }
	  s++;
     } 
     Stop_I2c();               

     return 1;
}

/* static unsigned char IRcvByte(unsigned char sla,unsigned char *c) */
/* { */
/*      Start_I2c();               */
   
/*      if ( 0 == SendByte(sla+1) ) */
/*      { */
/* 	  return (0); */
/*      } */
     
/*      *c=RcvByte();              */
   
/*      Ack_I2c(1);                */
/*      Stop_I2c();                */

/*      return(1); */
/* } */

/* Success: return 1
   Fail   : return 0 */
static unsigned char IRcvStr(unsigned char sla,unsigned char suba,unsigned char *s,unsigned char no)
{
     unsigned char i;
     
     Start_I2c();         
     if ( 0 == SendByte(sla) )
     {
	  return (0);
     }
     if ( 0 == SendByte(suba) )
     {
	  return (0);
     }

     Start_I2c();
     if ( 0 == SendByte(sla+1) )
     {
	  return (0);
     }
     

     for(i=0;i<no-1;i++)
     {   
	  *s=RcvByte();   
	  Ack_I2c(0);     
	  s++;
     } 
     *s=RcvByte();
     Ack_I2c(1);    
     Stop_I2c();    

     return(1);
}

/* Init line status and enable ds1307 */
static int simu_i2c_init(void)
{
     unsigned char reg0;

     at91_set_multi_drive(SDA, 1);
     at91_set_multi_drive(SCL, 1);

     if (!IRcvStr(0xd0, 0, &reg0, 1))
	  return -1;
     reg0 &= 0x7f;
     if (!ISendStr(0xd0, 0, &reg0, 1))
	  return -1;

     SET_SDA_OUT_HIGH;
     I2C_WAIT;
     SET_SCL_OUT_HIGH;
     I2C_WAIT;

     return 0;
}
  
static int read_rtc_status(char *buffer, char **start, off_t offset, int length, int *eof, void *data)
{
///     int i;
    unsigned char regs[7];
    int year, month, day, hour, min, sec, week;

    memset(regs, 0, sizeof (regs));
    if (!IRcvStr(0xd0, 0, regs, 7))
	printk("Read rtc error\n");

/*     for (i=0; i<7; i++)  */
/*      {  */
/* 	  printk ("%02x ", regs[i]);  */
/*      }  */
/*      printk("\n");  */

    year = 2000 + (regs[6] & 0x0f) + ((regs[6] & 0xf0) >> 4) * 10;
    month = (regs[5] & 0x0f) + ((regs[5] & 0x10)>>4) * 10;
    day = (regs[4] & 0x0f) + ((regs[4] & 0x30)>>4) * 10;
    week = regs[3] & 0x07;

    hour = (regs[2] & 0x0f) + ((regs[2] & 0x10)>>4) * 10 + ((regs[2] & 0x20)>>5) * 10;
    min = (regs[1] & 0x0f) + ((regs[1] & 0x70)>>4) * 10;
    sec = (regs[0] & 0x0f) + ((regs[0] & 0x70)>>4) * 10;

    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d %01d\n", \
	    year, month, day, hour, min, sec, week);
    return 23;
}

static int write_rtc_status(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
    char *buf;
    unsigned char regs[7];
    int year, month, day, week, hour, min, sec;

    memset(regs, 0, 7);
    buf = (char*)kmalloc(count, GFP_KERNEL);
    if (!buf)
	return -ENOMEM;

    if (copy_from_user(buf, buffer, count))
	return -EFAULT;

    sscanf(buf, "%04d-%02d-%02d %02d:%02d:%02d %01d", &year, &month, &day, &hour, &min, &sec, &week);

    regs[0] |= (sec % 10) | ((sec / 10) & 0x07)<<4;
    regs[1] |= (min % 10) | ((min / 10) & 0x07)<<4;
    if (hour>19)		/* The hour must be in 24 hour mode */
    {
	 regs[2] |= 0x20;
	 hour -= 10;
    }
    regs[2] |= (hour % 10) | ((hour / 10) & 0x01)<<4;
    regs[3] |= (week & 0x07);  	/* Monday;0 Tuesday:1 ... Sunday:7 */

    regs[4] |= (day % 10) | ((day / 10) & 0x03)<<4;
    regs[5] |= (month % 10) | ((month / 10) & 0x01)<<4;

    year -= 2000;
    regs[6] |= (year % 10) | ((year / 10) & 0x0f)<<4;

    if (!ISendStr(0xd0, 0, regs, 7))
	 printk("Set time error\n");

    kfree(buf);

    return count;    
}

#define BCD2BIN(val)  (((val) & 0x0f) + ((val)>>4)*10)
#define BIN2BCD(val)  ((((val)/10)<<4) + (val)%10)

static int gpio_get_time(struct device *dev, struct rtc_time *t)
{
    unsigned char regs[7];
    unsigned char tmp;
///    int i;

	memset(regs, 0, sizeof (regs));
	if (!IRcvStr(0xd0, 0, regs, 7))
		printk("Rtc get time error\n");

/*     for (i=0; i<8; i++) */
/*     { */
/* 	 printk ("%02x ", regs[i]); */
/*     } */
/*     printk("\n"); */

	t->tm_sec = BCD2BIN(regs[DS1307_REG_SECS] & 0x7f);
	t->tm_min = BCD2BIN(regs[DS1307_REG_MIN] & 0x7f);

	tmp = regs[DS1307_REG_HOUR] & 0x3f;
	t->tm_hour = BCD2BIN(tmp);

	t->tm_wday = BCD2BIN(regs[DS1307_REG_WDAY] & 0x07) - 1;
	t->tm_mday = BCD2BIN(regs[DS1307_REG_MDAY] & 0x3f);

	tmp = regs[DS1307_REG_MONTH] & 0x1f;
	t->tm_mon = BCD2BIN(tmp) - 1;

	/* assume 20YY not 19YY, and ignore DS1337_BIT_CENTURY */
	t->tm_year = BCD2BIN(regs[DS1307_REG_YEAR]) + 100;

	return 0;
}

static int gpio_set_time(struct device *dev, struct rtc_time *t)
{
	unsigned char regs[7];
	unsigned char tmp;

/*     printk("secs=%d, mins=%d, hours=%d, mday=%d, mon=%d, year=%d, wday=%d\n write", \ */
/* 	   t->tm_sec, t->tm_min,  t->tm_hour, t->tm_mday,\ */
/* 	   t->tm_mon, t->tm_year, t->tm_wday); */

	regs[DS1307_REG_SECS]  = BIN2BCD(t->tm_sec);
	regs[DS1307_REG_MIN]   = BIN2BCD(t->tm_min);
	regs[DS1307_REG_HOUR]  = BIN2BCD(t->tm_hour);
	regs[DS1307_REG_WDAY]  = BIN2BCD(t->tm_wday + 1);
	regs[DS1307_REG_MDAY]  = BIN2BCD(t->tm_mday);
	regs[DS1307_REG_MONTH] = BIN2BCD(t->tm_mon + 1);

    /* assume 20YY not 19YY */
	tmp = t->tm_year - 100;
	regs[DS1307_REG_YEAR] = BIN2BCD(tmp);

/*     printk("%02x %02x %02x %02x %02x %02x %02x write", \ */
/* 	   regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6]); */

	if (!ISendStr(0xd0, 0, regs, 7))
		printk("Rtc set time error\n");

	return 0;
}

static const struct rtc_class_ops gpio_rtc_ops =
{
	.read_time = gpio_get_time,
	.set_time  = gpio_set_time,
};

static int gpio_rtc_probe(struct platform_device *plat_dev)
{
	int err;
	struct rtc_device *rtc = rtc_device_register("gpio_rtc", &plat_dev->dev,
				  &gpio_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) 
	{
		err = PTR_ERR(rtc);
		return err;
	}
	platform_set_drvdata(plat_dev, rtc);
	return 0;
}

static int __devexit gpio_rtc_remove(struct platform_device *plat_dev)
{
	struct rtc_device *rtc = platform_get_drvdata(plat_dev);

	rtc_device_unregister(rtc);

	printk("%s driver removed\n", DEV_NAME);

	return 0;
}

static struct platform_driver gpio_rtc_drv = {
	.probe  = gpio_rtc_probe,
	.remove = __devexit_p(gpio_rtc_remove),
	.driver = {
		.name = "rtc-simu",
		.owner = THIS_MODULE,
	},
};

static int __init rtc_simu_init(void)
{
	int err;
	struct proc_dir_entry * res;

	if (simu_i2c_init() < 0)
		return -EFAULT;

	rtc_simu = proc_mkdir("rtc_simu", 0);
	if (!rtc_simu)
		return -ENOMEM;

	res = create_proc_entry("ds1307_regs", S_IRUGO | S_IWUGO, rtc_simu);
	if (!res)
		return -ENOMEM;
	res->read_proc  = read_rtc_status;
	res->write_proc = write_rtc_status;
	res->data = 0;

	if ((err = platform_driver_register(&gpio_rtc_drv)))
		return err;

	if ((gpio_rtc_dev = platform_device_alloc("rtc-simu", 0)) == NULL) 
	{
		err = -ENOMEM;
		goto exit_driver_unregister;
	}

	if ((err = platform_device_add(gpio_rtc_dev)))
		goto exit_free_dev;

	printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER,
		DRV_REVER_VER);

	return 0;

exit_free_dev:
	platform_device_put(gpio_rtc_dev);

exit_driver_unregister:
	platform_driver_unregister(&gpio_rtc_drv);
	return 0;
}

static void __exit rtc_simu_exit(void)
{
	remove_proc_entry("ds1307_regs", rtc_simu);
	remove_proc_entry("rtc_simu", 0); 

	platform_device_unregister(gpio_rtc_dev);
	platform_driver_unregister(&gpio_rtc_drv);
}

module_init(rtc_simu_init);
module_exit(rtc_simu_exit);
MODULE_LICENSE("GPL");

