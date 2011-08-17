ARCH = AT91SAM9260
PLAT ?= L200
HWVER ?= V5

KERNEL_VER = linux-3.0
#KERNEL_VER = linux-2.6.33
#KERNEL_VER = linux-2.6.22.1


LINUX_SRC ?= ../kernel/$(KERNEL_VER)/
CROSS_COMPILE ?= /opt/buildroot_350/build_arm/staging_dir/bin/arm-linux-uclibc-
INST_PATH=../rootfs/mnt/usr/drivers/

PWD := $(shell pwd)

EXTRA_CFLAGS+=-DPLAT_${PLAT}
EXTRA_CFLAGS+=-DARCH_${ARCH}
EXTRA_CFLAGS+=-DHWVER_${HWVER}
EXTRA_CFLAGS+=-D__KERNEL__

obj-m += dev_led.o
obj-m += dev_beep.o
obj-m += dev_faketty.o
obj-m += dev_eeprom.o
obj-m += dev_detect.o
obj-m += dev_rtc.o
obj-m += dev_sam.o
obj-m += dev_gpio.o
obj-m += dev_gprs.o
dev_gprs-objs  :=  dev_gprs_main.o gprs_lowlevel.o

modules:
	@make -C $(LINUX_SRC) M=$(PWD) modules
	@make clear
	cp dev_gprs.ko /tftpboot
	#@make install

uninstall:
	rm -f ${INST_PATH}/*.ko

install: uninstall
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

