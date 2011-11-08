
ARCH = s3c2440
KERNEL_VER = linux-3.0
LINUX_SRC ?= ../kernel/$(KERNEL_VER)
#INST_PATH=../rootfs/mnt/usr/drivers/
#INST_PATH=/tftp
INST_PATH=~


CROSS_COMPILE ?= /opt/buildroot-2011.02/arm926t/usr/bin/arm-linux-

PWD := $(shell pwd)

EXTRA_CFLAGS+=-DARCH_${ARCH}
EXTRA_CFLAGS+=-D__KERNEL__
EXTRA_CFLAGS+=-DMODULE

obj-m += s3c_led.o
#obj-m += s3c_button.o

modules:
	@make -C $(LINUX_SRC) M=$(PWD) modules
	@make clear
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
	@rm -f .*ko.cmd .*.o.cmd .*.o.d

clean: clear
	@rm -f  *.ko

distclean:  clear
	@rm -f cscope* tags
	@rm -f *.ko

