
PWD=$(shell pwd)

#If compiled by this makefile directly, then set the ARCH and PRJDIR. 
# Or it will read some enviorment from top makefile
#PRJDIR=$(shell dirname ${PWD})
#CFLAGS+=-I${PRJDIR}

INSTPATH=${PRJDIR}/.libs

VPATH= .
SRCS = $(wildcard ${VPATH}/*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

LIBNAME=$(shell basename ${PWD})
STALIB=lib${LIBNAME}.a
DYNLIB=lib${LIBNAME}.so

ifeq ($(ARCH), ARM7)
CROSS_COMPILE+=/usr/local/arm_tools_3.3/bin/arm-elf-
CC = ${CROSS_COMPILE}gcc
AR = ${CROSS_COMPILE}ar
PLATOS=UCLINUX
endif

ifeq ($(ARCH), ARM9)
CROSS_COMPILE=/opt/buildroot-eabi/staging/usr/bin/arm-linux-
CC = ${CROSS_COMPILE}gcc
AR = ${CROSS_COMPILE}ar
LIBS = ${STALIB} ${DYNLIB}
PLATOS=LINUX
endif

ifeq ($(PLATOS), UCLINUX)
LIBS = ${STALIB}
else
LIBS = ${STALIB} ${DYNLIB}
endif



all: ${LIBS}
	@make install
	@make clear

entry: 
	@echo "${INSTPATH}"
	@echo " "; 
	@echo " ** Compiling ${LIBNAME} library **"; 
	@echo " ";

${STALIB}:	$(OBJS) 
	@echo "  Making $@ ..."
	@$(AR) -rcu $@ $(OBJS)

${DYNLIB}:   $(OBJS) 
	@echo "  Making $@ ..."
	@$(CC) -fPIC -shared -o $@ $(OBJS)

%.o : %.c
	$(CC) -c $< $(CFLAGS)

tag: 
	@ctags --c-kinds=+defglmnstuvx --langmap=c:.c.h.ho.hem.het.hec.hev.him.hit.hic.hiv -R .  
	@cscope -Rbq

install:
	@cp ${LIBS} ${INSTPATH}

clear: 
	@rm -f *.o

clean: clear
	@rm -rf *.gdb *.a *.so
	@rm -f  tags cscope*

.PHONY:install clear clean entry
