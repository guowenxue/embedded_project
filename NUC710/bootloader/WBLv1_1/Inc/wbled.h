//----  LED on PORT 6
#define GPIO_CFG6       0xfff83060
#define GPIO_DIR6       0xfff83064
#define GPIO_DATAOUT6   0xfff83068
#define GPIO_DATAIN6    0xfff8306c

#define LED0 (1<<0)
#define LED1 (1<<1)
#define LED2 (1<<2)
#define LED3 (1<<3)
#define LED4 (1<<11)

//====  LED on PORT 6


//-------util PORT 0
#define GPIO_CFG0       0xfff83000
#define GPIO_DIR0       0xfff83004
#define GPIO_DATAOUT0   0xfff83008
#define GPIO_DATAIN0    0xfff8300c

#define RUN_LED (1<<2)

#define PWM_PPR  0xfff87000
#define PWM_CSR  0xfff87004
#define PWM_PCR  0xfff87008
#define PWM_CNR0 0xfff8700c
#define PWM_CMR0 0xfff87010
#define PWM_PDR0 0xfff87014
#define PWM_CNR1 0xfff87018
#define PWM_CMR1 0xfff8701c
#define PWM_PDR1 0xfff87020
#define PWM_PIER 0xfff8703c

//-------util PORT 0


// input/output to adr on int type: 32bit doble word
#define inpi(adr) ( *((unsigned int volatile *)(adr)) )
#define outpi(adr,val) ( *((unsigned int volatile *)(adr)) = val )

// input/output to adr on long type: 32bit doble word
#define inpl(adr) ( *((unsigned long volatile *)(adr)) )
#define outpl(adr,val) ( *((unsigned long volatile *)(adr)) = val )

void all_led_on(void);

void runled_blink(void);

void runled_on(void);

void start_buzzer(void);

void stop_buzzer(void);

void checkpass(void);

void checkfail(void);

void wbled(void);
