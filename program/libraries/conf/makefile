
#================================================================================================
#
# Description:  The makefile used to auto download libconfig source code(if it doesn't exist) and
#               compile to shared and static library, and install them to current folder
#     Version:  1.0.0
#   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
#   ChangeLog:  1, Release initial version on 2011.05.27
#
#================================================================================================

PWD=$(shell pwd)

ARCH ?= I386
CROSS_COMPILE ?=
INST_PATH=${PWD}

LIB_NAME=libconfig-1.4.7
DL_ADDR="http://www.hyperrealm.com/libconfig/${LIB_NAME}.tar.gz"

CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
LD=$(CROSS_COMPILE)ld
AR=$(CROSS_COMPILE)ar
NM=$(CROSS_COMPILE)nm
STRIP=$(CROSS_COMPILE)strip
RANLIB=$(CROSS_COMPILE)ranlib 
OBJDUMP=$(CROSS_COMPILE)objdump

CONFIG_OPTION=CC=${CC} CXX=${CXX} LD=${LD} AR=${AR} NM=${NM} STRIP=${STRIP} RANLIB=${RANLIB} OBJDUMP=${OBJDUMP}
ifneq (${CROSS_COMPILE},  )
	CONFIG_OPTION+=--host=arm-linux --target=arm-linux --build=i686
endif	

all:  libconfig_src libconfig_make

libconfig_make: ${LIB_NAME}
	cd ${LIB_NAME} ; \
		./configure ${CONFIG_OPTION} ; \
		make ; \
	cd - ; \
	make install 

libconfig_src: 
	@if (! test -s ext/${LIB_NAME}.tar.gz ) ; then \
		mkdir -p ext && cd ext ; \
		wget ${DL_ADDR} ; \
		cd - ; \
	fi;
	@if (! test -d ${LIB_NAME} ) ; then \
		tar -xzf ext/${LIB_NAME}.tar.gz ; \
	fi;

install:
	@cp -af ${LIB_NAME}/lib/.libs/libconfig.so* ${INST_PATH} ; \
	cp -af ${LIB_NAME}/lib/.libs/libconfig.a ${INST_PATH} ; \
	cp -af ${LIB_NAME}/lib/libconfig.h ${INST_PATH}

uninstall:
	@rm -f *.so*
	@rm -f *.a
	@rm -f *.h

clean: uninstall
	@if ( test -f  ${LIB_NAME}/Makefile ) ; then \
		make clean -C ${LIB_NAME} ; \
	fi

distclean: uninstall
	@if ( test -f  ${LIB_NAME}/Makefile ) ; then \
		make distclean -C ${LIB_NAME} ; \
	fi

clear: uninstall
	@rm -rf ${LIB_NAME}
