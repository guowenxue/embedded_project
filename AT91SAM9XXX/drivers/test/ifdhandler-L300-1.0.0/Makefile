# Muscle Smartcard Development
# Makefile
# David Corcoran

CROSS    = /opt/buildroot_350/build_arm/staging_dir/bin/arm-linux-
CC       = ${CROSS}gcc
AR       = ${CROSS}ar
LD       = ${CROSS}ld
LEX      = flex

CFLAGS   = -I. -DPCSC_DEBUG=1 -Werror #-DATR_DEBUG=1

MAKEXE   = make
LIBNAME  = libifdhdl.a
BINNAME  = ifdtest
PREFIX   = /usr/local/pcsc

all: extra

clean:
	rm -f *.o $(LIBNAME) core ${BINNAME}
extra: unix
	$(CC) ${CFLAGS} test.c -o ${BINNAME} -L. -lifdhdl
	@chmod 777 ${BINNAME}
	@rm -f *.o
	@rm -f ${LIBNAME}

unix: 
	$(CC) $(INCLUDE) $(CFLAGS) -c ifdhandler.c 
	$(AR) rcs ${LIBNAME} *.o 
	@rm -f *.o

install:
	cp -f ${LIBNAME} ../../libs
