#================================================================================================

# Description:  The makefile used to compiler all the source code in the folder and generate 
#               both shared and static library, which named lib${folder_name}.[a|so]
#     Version:  1.0.0
#   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
#   ChangeLog:  1, Release initial version on 2011.05.27
#
#================================================================================================

PWD=$(shell pwd)

ARCH ?= I386
CROSS_COMPILE ?= 
PLATOS ?= Linux

CC = ${CROSS_COMPILE}gcc
AR = ${CROSS_COMPILE}ar

LIBNAME=$(shell basename ${PWD})

STALIB=lib${LIBNAME}.a
DYNLIB=lib${LIBNAME}.so
LIBS = ${STALIB} ${DYNLIB}

VPATH= .
SRCS = $(wildcard ${VPATH}/*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

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
