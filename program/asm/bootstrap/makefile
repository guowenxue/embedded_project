
# ***********************************************************************
# *        File:  makefile
# *     Version:  1.0.0
# *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
# * Description:  Makefile used to cross compile the ASM source code
# *   ChangeLog:  1, Release initial version on "Sun Mar 20 18:41:04 CST 2011"
# *
# ***********************************************************************
#

APP_NAME=bootstrap
INST_PATH=~/winxp

BOARD=SMDK2440
#BOARD=SMDK2416

export BOARD

SRC_TOP=$(CURDIR)

CFLAGS+=-I${SRC_TOP}/include 
CFLAGS+=-Os -D__KERNEL__ -DCONFIG_ARM -D__ARM__ -DBOARD_$(BOARD)
PLATFORM_LIBS += -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc

ifeq ($(BOARD),SMDK2440)
TEXT_BASE=0x33f80000
CFLAGS+=-DTEXT_BASE=$(TEXT_BASE)
CROSS_COMPILE=/opt/buildroot-2011.02/arm920t/usr/bin/arm-linux-
CFLAGS+= -fno-strict-aliasing  -fno-common -ffixed-r8 -msoft-float #-mfloat-abi=soft
CFLAGS+=-march=armv4t -Wall -Wstrict-prototypes -fno-stack-protector
PLATFORM_LIBS += -lc -L/opt/buildroot-2011.02/arm920t/usr/arm-unknown-linux-uclibcgnueabi/sysroot/usr/lib/
endif

ifeq ($(BOARD),SMDK2416)
TEXT_BASE=0x33f80000
CFLAGS+=-DTEXT_BASE=$(TEXT_BASE)
CROSS_COMPILE=/opt/buildroot-2011.02/arm926t/usr/bin/arm-linux-
CFLAGS+= -fno-strict-aliasing  -fno-common -ffixed-r8 -msoft-float -mfloat-abi=soft
CFLAGS+=-march=armv4t -Wall -Wstrict-prototypes -fno-stack-protector
endif

AS  = $(CROSS_COMPILE)as
LD  = $(CROSS_COMPILE)ld
CC  = $(CROSS_COMPILE)gcc
CPP = $(CC) -E
AR  = $(CROSS_COMPILE)ar
NM  = $(CROSS_COMPILE)nm
LDR = $(CROSS_COMPILE)ldr
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB  = $(CROSS_COMPILE)RANLIB

gccincdir := $(shell $(CC) -print-file-name=include)
CFLAGS += -ffreestanding -nostdinc -isystem $(gccincdir) -pipe 

CFLAGS += -fno-builtin  
LDFLAGS=-Bstatic -T$(APP_NAME).lds -Ttext $(TEXT_BASE)
AFLAGS := $(CFLAGS) -D__ASSEMBLY__

export  CROSS_COMPILE AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP CFLAGS

SOBJS   = start.o mem_init.o
OBJS    = bootstrap.o board.o printf.o


		#
ALL: .depend $(SOBJS) $(OBJS)
	cd $(SRC_TOP) 
	${LD} $(LDFLAGS) $(SOBJS) \
	--start-group $(OBJS) \
	--end-group $(PLATFORM_LIBS) \
	-Map $(APP_NAME).map  -o $(APP_NAME).elf
	${OBJCOPY} -O binary $(APP_NAME).elf $(APP_NAME).bin
	@chmod 777 $(APP_NAME).bin
	@rm -f *.elf *.o
	@make install

.depend:    makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
	$(CC) -M $(AFLAGS) -D__ASSEMBLY__ $(SOBJS:.o=.S) > $@ 
	$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend

%.o: %.S
	 $(CC) $(AFLAGS) -c -o $@ $<
%.o: %.c
	 $(CC) $(CFLAGS) -c -o $@ $<

install: $(APP_NAME).bin
	cp $(APP_NAME).bin $(INST_PATH) -f --reply=yes

clean:
	@find $(OBJTREE) -type f \
			\( -name 'core' -o -name '*.bak' -o -name '*~' -o -name .depend \
			-o -name '*.o'  -o -name '*.a' -o -name '*.elf' \) -print \
			| xargs rm -f
	@rm -f $(APP_NAME).bin
	@rm -f $(APP_NAME).map

clear: clean
	@rm -f cscope.* tags


