
MAJOR=0
MINOR=0
REVER=1


PWD := $(shell pwd)
PRJ_PATH := $(shell dirname ${PWD})
INST_PATH:=/tmp/tftp
#CFLAGS+=-I${PRJ_PATH}/include 
#LDFLAGS= -L${PRJ_PATH}/libs 

#CROSS?=/opt/buildroot_350/build_arm/staging_dir/bin/arm-linux-
CC=${CROSS}gcc
AR=${CROSS}ar

VPATH= .
SRCS = $(wildcard ${VPATH}/*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

LIB_NAME=$(shell basename ${PWD})
TARGET=test

all: version binary

version: 
	@echo " "; 
	@echo " ========================================="; 
	@echo " **  Create Software Version Head File  **"; 
	@echo " ========================================="; 
	sh scripts/mkversion ${MAJOR} ${MINOR} ${REVER}

binary: $(OBJS) 
	$(CC) *.o -o ${TARGET}

%.o : %.c
	$(CC) -c $< $(CFLAGS)

.PHONY:install
install:
	@rm -rf ${INST_PATH}/${TARGET} 
	cp -f  ${TARGET} ${INST_PATH}

tag:
	@ctags --c-kinds=+defglmnstuvx --langmap=c:.c.h.ho.hem.het.hec.hev.him.hit.hic.hiv -R .
	@cscope -Rbq

clean:
	rm -rf *.o *.gdb *.a
	rm -f ${TARGET}
distclean:
	rm -f cscope.* tags

