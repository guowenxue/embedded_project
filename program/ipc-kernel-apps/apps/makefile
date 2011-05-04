CROSS_COMPILE?=
ARCH?=ARM9

#Binary file install path
INSTPATH=/tmp/tftp

#I prefer every Warning take as Error by gcc when compiling
CFLAGS+=-Werror -O3

#Use static library or shared library.Not define will use shared library
LDFLAGS+=-static

BINNAME= netlink-recv netlink-send

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
    CROSS_COMPILE=/opt/buildroot_350/build_arm/staging_dir/bin/arm-linux-
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

# The Finally target
all: entry 
	${CC} ${CFLAGS} -o netlink-recv  netlink-exam-user-recv.c
	${CC} ${CFLAGS} -o netlink-send  netlink-exam-user-send.c


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


.PHONY:  all entry tar tag install uninstall clean distclean
