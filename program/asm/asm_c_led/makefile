# ***********************************************************************
# *        File:  makefile
# *     Version:  1.0.0
# *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
# * Description:  Makefile used to cross compile the ASM and C source code
# *   ChangeLog:  1, Release initial version on "Mon Mar 21 21:09:52 CST 2011"
# *
# ***********************************************************************

CROSS = /opt/buildroot-2011.02/arm920t/usr/bin/arm-linux-
CFLAGS = 

led.bin: start.S led.c
	${CROSS}gcc $(CFLAGS) -c -o start.o start.S
	${CROSS}gcc $(CFLAGS) -c -o led.o led.c
	${CROSS}ld -Tled.lds start.o led.o -o led.elf
	${CROSS}objcopy -O binary -S led.elf led.bin
	rm -f *.elf *.o

install:
	cp led.bin ~/winxp -f --reply=yes

clean:
	rm -f *.elf *.o
	rm -f led.bin
