ARCH = AT91SAM9260
PLAT ?= GR01

CROSS_COMPILE ?= /opt/buildroot-2011.02/arm926ejs/usr/bin/arm-linux-
LINUX_SRC ?= ../../kernel/at91sam9260/linux-2.6.24/
INST_PATH=/tftpboot

PWD := $(shell pwd)

EXTRA_CFLAGS+=-DPLAT_${PLAT}
EXTRA_CFLAGS+=-DARCH_${ARCH}

obj-m += dev_skeleton.o
obj-m += dev_gprs.o
obj-m += dev_beep.o
dev_gprs-objs := dev_gprs_main.o gprs_lowlevel.o

modules:
	@make -C $(LINUX_SRC) M=$(PWD) modules
	@make clear
	@make install

uninstall:
	rm -f ${INST_PATH}/*.ko

install:
	cp -af *.ko ${INST_PATH}

tag:
	@ctags --c-kinds=+defglmnstuvx --langmap=c:.c.h.ho.hem.het.hec.hev.him.hit.hic.hiv -R .
	@cscope -Rbq

clear:
	@rm -f *.o *.cmd *.mod.c
	@rm -rf  *~ core .depend  .tmp_versions Module.symvers modules.order -f
	@rm -f .*ko.cmd .*.o.cmd

clean: clear
	@rm -f  *.ko

distclean:  clear
	@rm -f cscope* tags
	@rm -f *.ko

