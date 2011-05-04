CROSS_COMPILE?=
ARCH?=I386

#Program Version controled here
MAJOR=1
MINOR=0
REVER=0

BASEDIR=$(shell pwd)
PRJNAME=$(shell basename ${BASEDIR} )
TARBAR=${PRJNAME}.`date +%Y%m%d.%H%M%S`.tar.bz2 

#BINNAME is the C files(${BINNAME}.c) which should contained the main() function.
BINNAME = linker

#Binary file install path
INSTPATH=/tmp/tftp

CFLAGS+=-I${BASEDIR}

#I prefer every Warning take as Error by gcc when compiling
CFLAGS+=-Werror -O3

#Use static library or shared library.Not define will use shared library
LDFLAGS+=-static

# ====================================
# ** ARCH SETTINGS FOR EMBEDDED ARM **
# ====================================
ifeq ($(ARCH), ARM7)
    CROSS_COMPILE+=/usr/local/arm_tools_3.3/bin/arm-elf-
    STRIP=/usr/local/arm_tools_3.3/arm-elf/bin/flthdr -z -r
    CFLAGS+=--static
    CFLAGS+=-DUCLINUX
    CFLAGS+=-DFROM_SERVER
    LDFLAGS+=-Wl,-elf2flt="-s 16384"
    OS=UCLINUX
endif

ifeq ($(ARCH), ARM9)
    CROSS_COMPILE=/opt/buildroot-eabi/staging/usr/bin/arm-linux-
    STRIP=${CROSS_COMPILE}strip
    CFLAGS+=-DLINUX
    OS=LINUX
endif


ifeq ($(ARCH), I386)
    CROSS_COMPILE=
    STRIP=${CROSS_COMPILE}strip
    CFLAGS+=-DLINUX
endif

CFLAGS+=-DARCH_$(ARCH)
CFLAGS+=-DIS_LITTLE_ENDIAN -Wall -D_GNU_SOURCE -D_REENTRANT -D_LARGE_THREADS -D_THREAD_SAFE

#All the library should install to here (${BASEDIR}/libs).
LDFLAGS+=-L.libs

#The sub-folders need goes into compile
#SRCPATH = arch common eeprom comport gprs
SRCPATH = link
LDFLAGS+=-llink

export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export AR=${CROSS_COMPILE}ar
export AS=${CROSS_COMPILE}as
export NM=${CROSS_COMPILE}nm
export LD=${CROSS_COMPILE}ld
export RANLIB=${CROSS_COMPILE}ranlib
export STRIP
export LDFLAGS
export CFLAGS
export PRJDIR=${BASEDIR}
export PLATOS=${OS}

#A function used to goes into the sub-folder to compile
MAKEFUNC = @MakeSubDir() \
	{ \
		for DIR in `ls|grep -v 'libs'|grep -v 'include'|grep -v 'lib'|grep -v 'bin'`; do \
			if [ -d $${DIR} ] ; then \
				cd $${DIR}; \
				MakeSubDir; \
				if [ -f makefile -o -f Makefile ]; then \
					echo ""; \
					pwd; \
					make $(1); \
					if [ "$$?" != "0" ]; then \
						exit 1; \
					fi; \
				fi; \
				cd ..; \
			fi; \
		done; \
		if [ -f makefile -o -f Makefile ]; then \
			echo ""; \
			pwd; \
			make $(1); \
			if [ "$$?" != "0" ]; then \
				exit 1; \
			fi; \
		fi; \
	}; \

MAKEME = cd $(2); MakeSubDir $(1); cd ..;
LOOPMAKEFUNC = $(MAKEFUNC) $(foreach dir,$(SRCPATH),$(call MAKEME,$(1),$(dir)))

# The Finally target
all: entry version 
	@$(call LOOPMAKEFUNC,all)
	${CC} ${CFLAGS} -o ${BINNAME} ${BINNAME}.c ${LDFLAGS} 
	@echo " "; 
	@echo " ** Striping... **"; 
	@echo " "; 

	${STRIP} ${BINNAME} 
	@echo " "; 
	@echo " ** Done **"; 
	@echo " ";


# Do clean work first
entry: clean 
	@clear; 
	@echo " "; 
	@echo " =============================================="; 
	@echo " **  Lserials ${PRJNAME} program compilation    **"; 
	@echo " =============================================="; 
	@echo " ARCH: $(ARCH)"; 
	@echo " ";

	@echo " "; 
	@echo " ** Linking... **"; 
	@echo " "; 

#Generate the version.h head file by shell script
version: 
	@echo " "; 
	@echo " ========================================="; 
	@echo " **  Create Software Version Head File  **"; 
	@echo " ========================================="; 
	sh scripts/mkversion ${MAJOR} ${MINOR} ${REVER}

# Packet all the source code
tar:
	@tar -cjf ${TARBAR} *

# Make tags file for ctags and cscope, which used to read the source code.
tag:
	@ctags --c-kinds=+defglmnstuvx --langmap=c:.c.h.ho.hem.het.hec.hev.him.hit.hic.hiv -R .
	@@cscope -Rbq

install:
	@echo " ** Install \"${BINNAME}\" to \"${INSTPATH}\" ** "
	@cp ${BINNAME}  ${INSTPATH}

uninstall: 
	@echo " ** Uninstall \"${BINNAME}\" from \"${INSTPATH}\" ** " 
	@rm -rf ${INSTPATH}/${BINNAME}

clean:
	@$(call LOOPMAKEFUNC,clean)
	@rm -f libs/*.a
	@rm -f libs/*.so
	@find -iname "*.gdb" -exec rm -rf {} \;
	@rm -f ${BINNAME}

distclean:  clean 
	@rm -f cscope* tags


.PHONY:  all entry version tar tag install uninstall clean distclean
