INST_PATH = /tmp/tftp
ARCH?=i386

#LDFLAGS+=-lpthread
CFLAGS+=-Werror -Wall

DEBUG=1
ifdef DEBUG
	CFLAGS+=-g -DDEBUG
else
	CFLAGS+=-O2
	COMPRESS=$(STRIP)
endif

# ==================================
# ** ARCH SETTINGS FOR L100, L200 **
# ==================================

TMP=$(shell echo $(ARCH) | tr "[a-z]" "[A-Z]")
ifneq (,$(filter L100 L200,$(TMP)))
	CROSS_COMPILE=/usr/local/arm_tools_3.3/bin/arm-elf-
	CONFIGURE_ARCH=--host=arm-elf --build=i386
	COMPRESS=flthdr -z -r -s 16384
	CFLAGS+=-DARCH_$(ARCH)
	CFLAGS+=-DUCLINUX -DIS_LITTLE_ENDIAN -D_GNU_SOURCE -D_REENTRANT -D_LARGE_THREADS -D_THREAD_SAFE
	LDFLAGS+=-Wl,-elf2flt
	PLATOS=UCLINUX
endif

# ==================================
# ** ARCH SETTINGS FOR L300, L350 **
# ==================================

TMP=$(shell echo $(ARCH) | tr "[a-z]" "[A-Z]")
ifneq (,$(filter L300 L350 N300,$(TMP)))
	CROSS_COMPILE=arm-linux-uclibc-
	CONFIGURE_ARCH=--host=arm-linux --build=i386
	CFLAGS+=-DARCH_$(ARCH)
	CFLAGS+=-DLINUX -DIS_LITTLE_ENDIAN -D_GNU_SOURCE -D_REENTRANT -D_LARGE_THREADS -D_THREAD_SAFE
	LDFLAGS+=-ldl
	PLATOS=LINUX
endif

# ============================
# ** ARCH SETTINGS FOR I386 **
# ============================

TMP=$(shell echo $(ARCH) | tr "[a-z]" "[A-Z]")
ifneq (,$(filter I386,$(TMP)))
	CROSS_COMPILE=
	CONFIGURE_ARCH=
	CFLAGS+=-DARCH_$(ARCH)
	LDFLAGS+=-ldl
	CFLAGS+=-DLINUX -DIS_LITTLE_ENDIAN
	PLATOS=LINUX
endif

export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export AS=${CROSS_COMPILE}as
export RANLIB=${CROSS_COMPILE}ranlib
export STRIP=${CROSS_COMPILE}strip
export PLATOS
export CONFIGURE_ARCH
export CFLAGS
export LDFLAGS
export ARCH

SRCFILES = $(wildcard *.c)
BINARIES=$(SRCFILES:%.c=%.exe)

all: entry binaries

entry:
	@echo " ";
	@echo " =========================================================";
	@echo " ** Compile source code for ARCH $(ARCH) on $(PLATOS)";
	@echo " =========================================================";
	@echo " ";

binaries:  ${BINARIES}
	@echo " Compile over"

%.exe: 	%.c
	$(CC) -o $@ $< $(CFLAGS)
	
tag:
	@cscope -Rbq
	@ctags --c-kinds=+defglmnstuvx --langmap=c:.c.h.ho.hem.het.hec.hev.him.hit.hic.hiv -R .

install:
	cp $(BINARIES) ${INST_PATH}

uninstall:
	@cd ${INST_PATH} && rm -f $(BINARIES) && cd -

clean:
	@rm -f *.o $(BINARIES)
	@rm -f *.elf*
	@rm -f *.gdb

distclean: clean
	@rm -f tags  cscope*

.PHONY: all

