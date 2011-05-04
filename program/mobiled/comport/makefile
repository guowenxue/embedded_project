
PWD=$(shell pwd)

#If wanna compile in the subdir, not called by top makefile, uncomment it
#LOCAL_COMPILE=YES
#PLAT?=L200


LIBNAME=$(shell basename ${PWD})
STALIB=lib${LIBNAME}.a
DYNLIB=lib${LIBNAME}.so

VPATH= .
SRCS = $(wildcard ${VPATH}/*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))


#======================================================
#  ---> Doesn't call by top makefile, compile by local
#======================================================
ifeq (${LOCAL_COMPILE}, YES)
PRJDIR?=$(shell dirname ${PWD})
CFLAGS+=-I${PRJDIR}

ifneq (,$(filter L200 L100,$(PLAT)))
CROSS_COMPILE?=/usr/local/arm_tools_3.3/bin/arm-elf-
CC = ${CROSS_COMPILE}gcc
AR = ${CROSS_COMPILE}ar
CFLAGS += -DUCLINUX
CFLAGS += -DARCH_W90P710
PLATOS=UCLINUX
endif

ifneq (,$(filter L300 L350 N300 L200_ARM9,$(PLAT)))
CROSS_COMPILE=/opt/buildroot_350/build_arm/staging_dir/bin/arm-linux-
CC = ${CROSS_COMPILE}gcc
AR = ${CROSS_COMPILE}ar
LIBS = ${STALIB} ${DYNLIB}
CFLAGS += -DLINUX
CFLAGS += -DARCH_AT91SAM9260
PLATOS=LINUX
endif
endif #End local compile

ifeq ($(PLATOS), UCLINUX)
LIBS = ${STALIB}
else
LIBS = ${STALIB} #${DYNLIB}
endif

all: entry ${LIBS}

entry: 
	@echo " ";
	@echo " =========================================================";
	@echo " ** Compile subdir ${LIBNAME} for ${ARCH} on ${PLATOS} "
	@echo " =========================================================";

${STALIB}:	$(OBJS) 
	@$(AR) -rcu $@ $(OBJS)

${DYNLIB}:   $(OBJS) 
	@$(CC) -fPIC -shared -o $@ $(OBJS)

%.o : %.c
	$(CC) -c $< $(CFLAGS)

tag: 
	@ctags --c-kinds=+defglmnstuvx --langmap=c:.c.h.ho.hem.het.hec.hev.him.hit.hic.hiv -R .  
	@cscope -Rbq

install:
	@mv ${LIBS} ${INSTPATH}

clean:
	@rm -f *.o
	@rm -rf *.gdb *.a *.so

distclean: clean
	@rm -f  tags cscope*

.PHONY: clean entry
